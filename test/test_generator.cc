#include "test_generator.h"

#include <format>
#include <iostream>

libco::SimpleGenerator<int> Fib(int n) {
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

struct A {
  int val;
  ~A() { std::cout << "~A()\n"; }
};

libco::SimpleGenerator<A> YieldOnce() {
  // notice A's lifetime
  co_yield A{1};
}

void TestFib(int n) {
  auto g = Fib(n);
  for (int i = 0; const auto& j : g) {
    std::cout << std::format("Fib({})={} \n", i++, j);
  }
}

void TestYieldOnce() {
  auto g = YieldOnce();
  // https://zh.cppreference.com/w/cpp/language/range-for
  for (int i = 0; const auto& j : g) {
    std::cout << std::format("YieldOnce {} {} \n", i++, j.val);
  }
}

libco::RecursiveGenerator<int> Fib1(int n) {
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

libco::RecursiveGenerator<int> Fib2(int n) {
  co_yield Fib1(n);
  co_yield Fib1(n);
}

void TestFib2(int n) {
  auto g = Fib2(n);
  for (int i = 0; const auto& j : g) {
    std::cout << std::format("Fib({})={} \n", i++, j);
  }
}
