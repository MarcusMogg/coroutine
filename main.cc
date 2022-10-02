#include <iostream>
#include <string>

#include "src/co_stackful.h"

void Test(const int times, const std::string& s) {
  for (int i = 0; i < times; i++) {
    std::cout << s << "\n";
    libco::CoYield();
  }
}

int main() {
  std::string a = "a";
  // std::string b = "b";

  const auto co1 = libco::CoStart("co1", []() { Test(1, "a"); });
  // const auto co2 = libco::CoStart("co2", [&]() { Test(6, b); });

  libco::CoWait(co1);
  std::cout << "----------\n";
  // libco::CoWait(co2);

  return 0;
}
