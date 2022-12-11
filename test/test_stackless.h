#pragma once

#include <format>
#include <string>

#include "src/co_stackless.h"

class RequestLineParser : public libco::StackLessCo {
 public:
  bool Consume(char c);

  const std::string ToString() const {
    return std::format("method:{} \npath:{} \nversion:{}", method_, path_, version_);
  }

 private:
  std::string method_;
  std::string path_;
  int index_ = 0;
  int version_ = 0;  // 10 or 11
};

void TestStackLess();
