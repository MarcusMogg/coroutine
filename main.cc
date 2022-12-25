#include <iostream>
#include <string>

#include "test/test_generator.h"
#include "test/test_stackless.h"

int main() {
  TestStackLess();
  TestFib(12);
  TestFib2(10);
  TestYieldOnce();
  return 0;
}
