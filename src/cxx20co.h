// #pragma once

// #include <coroutine>
// #include <cstddef>
// #include <cstdint>
// #include <memory>

// namespace libco {

// class IScheduler {
//  public:
//   virtual void push_back(std::coroutine_handle<> h);
//   virtual std::coroutine_handle<> pop_front();

//   virtual void run();

//   virtual ~IScheduler();
// };

// template <int N>
// class Scheduler : public IScheduler {
//  public:
//   void push_back(std::coroutine_handle<> h) override {
//     arr[head] = h;
//     head = (head + 1) % N;
//   }

//   std::coroutine_handle<> pop_front() override {
//     auto result = arr[tail];
//     tail = (tail + 1) % N;
//     return result;
//   }

//   auto try_pop_front() {
//     if (head == tail) {
//       return std::coroutine_handle<>{};
//     }
//     return pop_front();
//   }

//   void run() override {
//     while (auto h = try_pop_front()) {
//       h.resume();
//     }
//   }

//   ~Scheduler() {}

//  private:
//   std::size_t head = 0;
//   std::size_t tail = 0;
//   std::coroutine_handle<> arr[N];
// };

// template <typename T>
// class PrefetchAwaitable {
//  public:
//   PrefetchAwaitable(T& value, IScheduler* scheduler) : value(value), scheduler(scheduler) {}

//   bool await_ready() { return false; }
//   T& await_resume() { return value; }

//   auto await_suspend(std::coroutine_handle<> h) {
//     scheduler->push_back(h);
//     return scheduler->pop_front();
//   }

//  private:
//   T& value;
//   IScheduler* scheduler;
// };

// class BatchTaskPool {
//  public:
//   explicit BatchTaskPool(std::size_t limit) : limit(limit) {
//     scheduler = std::make_unique<Scheduler<16>>();
//   }
//   void Add(RootTask t) {
//     if (limit == 0) {
//       scheduler->pop_front().resume();
//     }
//     auto h = t.set_owner(this);
//     scheduler->push_back(h);
//     --limit;
//   }

//   void on_task_done() { ++limit; }
//   void join() { scheduler->run(); }
//   ~BatchTaskPool() { join(); }

//  private:
//   std::size_t limit;
//   std::unique_ptr<IScheduler> scheduler;
// };

// class RootTask {
//  public:
//   struct promise_type {  // required
//     BatchTaskPool* owner_;

//     std::suspend_always initial_suspend() { return {}; }
//     std::suspend_never final_suspend() noexcept {
//       owner_->on_task_done();
//       return {};
//     }
//     void unhandled_exception() {
//       std::terminate();
//     }  // saving
//        // exception

//     template <std::convertible_to<T> From>  // C++20 concept
//     std::suspend_always yield_value(From&& from) {
//       value_ = std::forward<From>(from);  // caching the result in promise
//       return {};
//     }
//     void return_void() {}
//   };

//   auto set_owner(BatchTaskPool* owner) {
//     auto result = h;
//     h.promise().owner_ = owener;
//     h = nullptr;
//     return result;
//   }
//   ~RootTask() {
//     if (h) {
//       h.destroy();
//     }
//   }

//  private:
//   std::coroutine_handle<promise_type> h;
// }

// }  // namespace libco