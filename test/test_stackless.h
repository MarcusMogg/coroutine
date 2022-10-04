#pragma once

#include <string>

#include "src/co_stackless.h"

class RequestLineParser : public libco::StackLessCo {
 public:
  bool Consume(char c);

  const std::string ToString() const {
    return std::string("method: ") + method_ + "\npath: " + path_ +
           "\nversion: " + std::to_string(version_);
  }

 private:
  std::string method_;
  std::string path_;
  int index_ = 0;
  int version_ = 0;  // 10 or 11
};

void TestStackLess();
