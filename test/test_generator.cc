#include "test_generator.h"

#include <iostream>

libco::Generator<int> Fib(int n) {
  if (n == 0 || n == 1) {
    co_return;
  }
  int a = 0, b = 1;
  co_yield a;
  co_yield b;

  for (int i = 2; i <= n; ++i) {
    co_yield a + b;

    b = a + b;
    a = b - a;
  }
  co_return;
}
libco::Generator<int> YieldOnce() { co_yield 1; }

void TestFib(int n) {
  auto g = Fib(n);
  for (int i = 0; const auto& j : g) {
    std::cout << "Fib(" << i++ << ")=" << j << "\n";
  }
}

void TestYieldOnce() {
  auto g = YieldOnce();
  for (int i = 0; const auto& j : g) {
    std::cout << "YieldOnce " << i++ << " " << j << "\n ";
  }
}