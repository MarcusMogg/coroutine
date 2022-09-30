#include "co_stackful.h"

#include <cassert>
#include <map>
#include <optional>
#include <string>

namespace libco {
static std::map<std::string, std::shared_ptr<StackFullCo>> _coroutines;
static std::shared_ptr<StackFullCo> _current = nullptr;

namespace detail {

void CoCall(const StackFullCo* co) { co->func(); }

void CoAdd(const std::shared_ptr<StackFullCo>& co) { _coroutines.emplace(co->name, co); }
void CoDelete(const std::shared_ptr<StackFullCo>& co) { _coroutines.erase(co->name); }

static inline void StackSwitchCall(void* sp, void* entry, uintptr_t arg) {
  /*
    *(sp-1) = *rsp
    rsp = sp - 1
    set arg
    call func
   */
  asm volatile(
      "movq %%rsp, -0x10(%0);"
      "leaq -0x10(%0), %%rsp;"
      "movq %2, %%rdi; call *%1"
      :
      : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
      : "memory");
}

static inline void StackRevert(void* sp) {
  // move old rsp to rsp from sp-1
  asm volatile("movq -0x10(%0),%%rsp;" : : "b"((uintptr_t)sp) : "memory");
}

}  // namespace detail

std::shared_ptr<StackFullCo> CoStart(const std::string& name, const FuncType& func) {
  std::shared_ptr<StackFullCo> co = std::make_shared<StackFullCo>(name, func);
  detail::CoAdd(co);
  return co;
}

void CoYield() {
  assert(_current != nullptr);
  int ret = setjmp(_current->context);
  if (ret) {
  } else {
    return;
  }
}

void CoWait(const std::shared_ptr<StackFullCo>& co) {
  while (co->status != CoStatus::DEAD) {
    CoYield();
  }

  detail::CoDelete(co);
}

}  // namespace libco