#pragma once

#include <algorithm>
#include <csetjmp>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace libco {
// 128kb
static constexpr int kStackSize = 128 * 1024;

// using FuncType = void(void*);
using FuncType = std::function<void()>;

enum class CoStatus : uint32_t {
  NEW = 1,  // 新创建，还未执行过
  RUNNING,  // 已经执行过
  WAITING,  // 在 co_wait 上等待
  DEAD,     // 已经结束，但还未释放资源
};

struct StackFullCo {
  const FuncType func;
  const std::string name;
  StackFullCo* waiter;        // 是否有其他协程在等待当前协程
  jmp_buf context;            // 寄存器现场 (setjmp.h)
  CoStatus status;            // 协程的状态
  uint8_t stack[kStackSize];  // 协程的堆栈

  StackFullCo(const std::string& name, FuncType&& func)
      : func(std::forward<FuncType>(func)), name(name), waiter(nullptr), status(CoStatus::NEW) {
    std::fill(stack, stack + kStackSize, 0);
  }
};

StackFullCo* CoStart(const std::string& name, FuncType&& func);
void CoYield();
void CoWait(const StackFullCo* co);

}  // namespace libco