#include "co_stackful.h"

#include <cassert>
#include <csetjmp>
#include <iostream>
#include <map>
#include <string>
#include <utility>
namespace libco {
__attribute__((init_priority(101))) static std::map<std::string, std::shared_ptr<StackFullCo>>
    _coroutines;
__attribute__((init_priority(102))) static std::shared_ptr<StackFullCo> _current = nullptr;

static constexpr char kMaster[] = "master";

namespace detail {

void CoCall(const StackFullCo* co) {
  co->func();
  std::cout << co->name << " end\n";
}

void CoAdd(const std::shared_ptr<StackFullCo>& co) { _coroutines.emplace(co->name, co); }
void CoDelete(const std::shared_ptr<StackFullCo>& co) { _coroutines.erase(co->name); }

std::shared_ptr<StackFullCo> CoNext() {
  for (const auto& [k, v] : _coroutines) {
    if (_current != nullptr && _current->name == k) {
      continue;
    }
    if (v->status == CoStatus::DEAD) {
      continue;
    }
    return v;
  }
  return _coroutines[kMaster];
}

static inline void StackSwitchCall(uintptr_t sp, uintptr_t entry, const uintptr_t arg) {
  /*
    *(sp-1) = *rsp
    rsp = sp - 1
    set arg
    call func
   */
  asm volatile(
      "movq %%rsp, -0x10(%0);"
      "leaq -0x20(%0), %%rsp;"
      "movq %2, %%rdi;"
      "call *%1;"
      "movq -0x10(%0),%%rsp;"
      :
      : "b"(sp), "d"(entry), "a"(arg)
      : "memory");
}

void CoSwitch(const std::shared_ptr<StackFullCo>& next_co) {
  if (next_co == nullptr) {
    return;
  }
  switch (next_co->status) {
    case CoStatus::NEW:
      if (_current) {
        _current->status = CoStatus::WAITING;
      }
      _current = next_co;
      _current->status = CoStatus::RUNNING;

      StackSwitchCall(
          reinterpret_cast<uintptr_t>(_current->stack + kStackSize),
          reinterpret_cast<uintptr_t>(CoCall),
          reinterpret_cast<const uintptr_t>(_current.get()));
      // CoCall end
      _current->status = CoStatus::DEAD;
      _current = _coroutines[kMaster];
      break;
    case CoStatus::WAITING:
      if (_current) {
        _current->status = CoStatus::WAITING;
      }
      _current = next_co;
      _current->status = CoStatus::RUNNING;
      longjmp(_current->context, 1);
      break;
    case CoStatus::DEAD:
      break;
    case CoStatus::RUNNING:
      break;
  }
}

__attribute__((constructor(201))) void BeforeMain() {
  const auto co1 = libco::CoStart(kMaster, []() {});
  _current = co1;
  _current->status = CoStatus::RUNNING;
}

}  // namespace detail

std::shared_ptr<StackFullCo> CoStart(const std::string& name, FuncType&& func) {
  std::shared_ptr<StackFullCo> co =
      std::make_shared<StackFullCo>(name, std::forward<FuncType>(func));
  detail::CoAdd(co);
  return co;
}

void CoYield() {
  std::shared_ptr<StackFullCo> next_co = nullptr;
  int ret = setjmp(_current->context);
  if (ret == 0) {
    next_co = detail::CoNext();
  } else {
    return;
  }

  detail::CoSwitch(next_co);
}

void CoWait(const std::shared_ptr<StackFullCo>& co) {
  while (co->status != CoStatus::DEAD) {
    CoYield();
  }

  detail::CoDelete(co);
}

}  // namespace libco