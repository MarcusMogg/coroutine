#include <iostream>
#include <string>

#include "src/co_stackful.h"

void Test(const std::string& s) {
  std::cout << s << "\n";
  libco::CoYield();
  while (1) {
    std::cout << s << "\n";
    libco::CoYield();
  }
}

int main() {
  std::string a = "a";
  std::string b = "b";

  const auto co1 = libco::CoStart("co1", [&]() { Test(a); });
  const auto co2 = libco::CoStart("co2", [&]() { Test(b); });

  libco::CoWait(co1);  // never returns
  libco::CoWait(co2);

  return 0;
}
