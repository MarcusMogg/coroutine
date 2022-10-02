#include "co_stackful.h"

#include <algorithm>
#include <cassert>
#include <csetjmp>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
namespace libco {
__attribute__((init_priority(101))) static std::map<std::string, std::unique_ptr<StackFullCo>>
    _coroutines;
static StackFullCo* volatile _current = nullptr;

static constexpr char kMaster[] = "master";

namespace detail {

void CoCall(const StackFullCo* co) {
  co->func();
  std::cout << co->name << " end\n";
}

StackFullCo* CoAdd(std::unique_ptr<StackFullCo>&& co) {
  const auto& it = _coroutines.emplace(co->name, std::move(co));
  return it.first->second.get();
}
void CoDelete(const StackFullCo* co) { _coroutines.erase(co->name); }

StackFullCo* CoNext() {
  for (const auto& [k, v] : _coroutines) {
    if (_current != nullptr && _current->name == k) {
      continue;
    }
    if (v->status == CoStatus::DEAD) {
      continue;
    }
    return v.get();
  }
  return nullptr;
}

static inline void StackSwitchCall(uintptr_t sp, uintptr_t entry, uintptr_t arg) {
  std::cout << sp << "\n";
  /*
    *(sp-1) = *rsp
    rsp = sp - 1
    set arg
    call func
    move old rsp to rsp from sp-1
   */
  asm volatile(
      "movq %%rsp, -0x10(%0);"
      "leaq -0x20(%0), %%rsp;"
      "movq %2, %%rdi;"
      "call %1;"
      "movq -0x10(%0), %%rsp;"
      :
      : "b"(sp), "d"(entry), "a"(arg)
      : "memory");
}

__attribute__((constructor(201))) void BeforeMain() {
  _current = libco::CoStart(kMaster, []() {});
  _current->status = CoStatus::RUNNING;
}

}  // namespace detail

StackFullCo* CoStart(const std::string& name, FuncType&& func) {
  std::unique_ptr<StackFullCo> co =
      std::make_unique<StackFullCo>(name, std::forward<FuncType>(func));
  return detail::CoAdd(std::move(co));
}

void CoYield() {
  int ret = setjmp(_current->context);
  if (ret == 0) {
    const auto next_co = detail::CoNext();
    if (next_co == nullptr) {
      return;
    }
    const auto cur_copy = _current;
    _current->status = CoStatus::WAITING;
    _current = next_co;

    if (next_co->status == CoStatus::NEW) {
      next_co->status = CoStatus::RUNNING;
      detail::StackSwitchCall(
          reinterpret_cast<uintptr_t>((next_co->stack) + kStackSize),
          reinterpret_cast<uintptr_t>(&detail::CoCall),
          reinterpret_cast<uintptr_t>(next_co));

      // CoCall end
      std::cout << " return\n";
      next_co->status = CoStatus::DEAD;
      _current = cur_copy;
      _current->status = CoStatus::RUNNING;
      // CoYield();
    } else if (next_co->status == CoStatus::WAITING) {
      next_co->status = CoStatus::RUNNING;
      longjmp(next_co->context, 1);
    }
  } else {
    return;
  }
}

void CoWait(const StackFullCo* co) {
  while (co->status != CoStatus::DEAD) {
    CoYield();
  }

  detail::CoDelete(co);
}

}  // namespace libco