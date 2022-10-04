#pragma once

#include <coroutine>
#include <memory>

namespace libco {

template <typename T>
class Generator {
 public:
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {
    T value_;

    auto co() { return handle_type::from_promise(*this); }
    Generator get_return_object() { return Generator(this); }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { throw; }

    template <std::convertible_to<T> From>
    std::suspend_always yield_value(From&& from) {
      value_ = std::forward<From>(from);
      return {};
    }
    void return_void() {}
  };

  explicit Generator(promise_type* p) : p_(p) {}
  ~Generator() {
    if (p_) {
      p_->co().destroy();
    }
  }

  struct GeneratorEnd {};
  struct GeneratorIter {
    bool operator!=(const GeneratorEnd&) { return !p->co().done(); }
    void operator++() { p->co()(); }
    T& operator*() { return p->value_; }

    promise_type* p;
  };
  auto end() { return GeneratorEnd{}; }
  auto begin() {
    auto it = GeneratorIter{p_};
    if (!begin_) {
      ++it;
      begin_ = true;
    }
    return it;
  }

 private:
  promise_type* p_ = nullptr;
  bool begin_ = false;
};

}  // namespace libco