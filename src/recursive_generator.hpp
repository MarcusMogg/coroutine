#pragma once

#include <coroutine>
#include <iterator>
#include <memory>
#include <utility>

namespace libco {

template <typename T>
class RecursiveGenerator {
 public:
  class promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  class promise_type {
   public:
    promise_type() : parent_(nullptr), root_(this), leaf_(this) {}
    auto co() { return handle_type::from_promise(*this); }
    auto get_return_object() { return RecursiveGenerator(co()); }
    std::suspend_always initial_suspend() { return {}; }
    auto final_suspend() noexcept {
      struct FinalAwaitable {
        bool await_ready() noexcept { return false; }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept {
          auto& promise = h.promise();
          auto parent = h.promise().parent_;
          if (parent) {
            promise.root_->leaf_ = parent;
            return handle_type::from_promise(*parent);
          }
          return std::noop_coroutine();
        }
        void await_resume() noexcept {}
      };
      return FinalAwaitable{};
    }
    void unhandled_exception() { throw; }

    std::suspend_always yield_value(T& value) {
      root_->value_ = std::addressof(value);
      return {};
    }
    std::suspend_always yield_value(T&& value) {
      root_->value_ = std::addressof(value);
      return {};
    }
    auto yield_value(RecursiveGenerator&& g) {
      struct YieldAwaitable {
        YieldAwaitable(RecursiveGenerator&& g) : g_(std::forward<RecursiveGenerator>(g)) {}

        bool await_ready() noexcept { return !g_.h_; }
        std::coroutine_handle<> await_suspend(handle_type h) noexcept {
          auto& current = h.promise();
          auto& nested = g_.h_.promise();
          auto& root = current.root_;
          // merge g to h
          nested.parent_ = &current;
          nested.root_ = root;
          root->leaf_ = &nested;

          return g_.h_;
        }
        void await_resume() noexcept {}

        RecursiveGenerator g_;
      };
      return YieldAwaitable(std::move(g));
    }
    void return_void() {}

    T& value() { return *root_->value_; }

    void resume() { handle_type::from_promise(*leaf_).resume(); }

   private:
    promise_type* parent_;
    promise_type* root_;
    promise_type* leaf_;

    std::add_pointer_t<T> value_;
  };

  RecursiveGenerator(handle_type h) : h_(h) {}

  struct iterator_end {};

  struct iterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::remove_reference_t<T>;
    using reference = std::conditional_t<std::is_reference_v<T>, T, T&>;
    using pointer = std::add_pointer_t<T>;

    bool operator!=(const iterator_end&) { return !h.done(); }
    void operator++() { h.promise().resume(); }
    value_type& operator*() { return h.promise().value(); }

    handle_type h;
  };

  auto end() const { return iterator_end{}; }
  auto begin() {
    auto it = iterator{h_};
    if (!begin_) {
      ++it;
      begin_ = true;
    }
    return it;
  }

 private:
  handle_type h_;
  bool begin_ = false;
};

}  // namespace libco