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

void CoCall(StackFullCo* co) {
  co->func();
  co->status = CoStatus::DEAD;
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
    const auto volatile next_co = detail::CoNext();
    if (next_co == nullptr) {
      return;
    }
    _current->status = CoStatus::WAITING;
    _current = next_co;

    if (next_co->status == CoStatus::NEW) {
      next_co->status = CoStatus::RUNNING;
      asm volatile(
          "movq %%rsp, 0(%0);"
          "movq %0, %%rsp;"
          "movq %2, %%rdi;"
          "call *%1;"
          "movq 0(%0), %%rsp;"
          :
          : "b"(reinterpret_cast<uintptr_t>((next_co->stack) + kStackSize) - 16),
            "d"(reinterpret_cast<uintptr_t>(&detail::CoCall)),
            "a"(reinterpret_cast<uintptr_t>(next_co)));

      // CoCall end
      std::cout << " return\n";

      CoYield();
    } else if (next_co->status == CoStatus::WAITING) {
      next_co->status = CoStatus::RUNNING;
      longjmp(next_co->context, 1);
    }
  } else {
    return;
  }
}

void CoWait(StackFullCo* co) {
  while (co->status != CoStatus::DEAD) {
    CoYield();
  }

  detail::CoDelete(co);
}

}  // namespace libco