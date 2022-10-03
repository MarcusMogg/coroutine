#include <iostream>
#include <string>

#include "src/co_stackless.h"

#define reenter(c) CO_BEGIN(c)
#define yield CO_YIELD

using namespace std;

class RequestLineParser : public libco::StackLessCo {
 public:
  bool Consume(char c) {
    reenter(this) {
      // 读取method
      while (isalpha(c)) {
        method_.push_back(c);
        yield true;
      }
      // 读完了method
      if (method_.empty()) {
        cout << "没有method，格式错误。\n";
        yield false;
      }
      // 读取空格
      if (c != ' ') {
        cout << "method之后不是空格，格式错误。\n";
        yield false;
      } else {
        yield true;
      }
      // 读取路径
      while (std::isalpha(c) || c == '/') {
        path_.push_back(c);
        yield true;
      }
      if (path_.empty()) {
        cout << "path为空，格式错误\n";
        yield false;
      }
      // 读取空格
      if (c != ' ') {
        cout << "path之后不是空格，格式错误。\n";
        yield false;
      } else {
        yield true;
      }
      // 读取HTTP
      if (c != 'H') {
        cout << "第一个H错误\n";
        yield false;
      }
      yield true;
      if (c != 'T') {
        cout << "第1个T错误\n";
        yield false;
      }
      yield true;
      if (c != 'T') {
        cout << "第2个T错误\n";
        yield false;
      }
      yield true;
      if (c != 'P') {
        cout << "P错误\n";
        yield false;
      }
      yield true;
      if (c != '/') {
        cout << "HTTP之后的/错误\n";
        yield false;
      }
      yield true;
      // 读取1.0或1.1
      if (c != '1') {
        cout << "读取1错误\n";
        yield false;
      }
      yield true;
      if (c != '.') {
        cout << "读取.错误\n";
        yield false;
      }
      yield true;
      if (c != '1' && c != '0') {
        cout << "读取1/0错误";
        yield false;
      } else {
        version_ = 10 + c - '0';
        yield true;
      }
    }
    return false;
  }

  string ToString() const {
    return string("method: ") + method_ + "\npath: " + path_ + "\nversion: " + to_string(version_);
  }

 private:
  std::string method_;
  std::string path_;
  int index_ = 0;
  int version_ = 0;  // 10 or 11
};

int main() {
  std::string test = "GET /root/path HTTP/1.1";
  RequestLineParser p;
  for (char c : test) {
    if (!p.Consume(c)) {
      cout << "failed to parse\n";
      return -1;
    }
  }
  // 解析成功了，输出看看
  cout << p.ToString() << "\n";

  return 0;
}
