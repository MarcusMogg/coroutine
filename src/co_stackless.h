#pragma once

namespace libco {

static constexpr int kInit = 0;
static constexpr int kExit = -1;

class StackLessCo {
 public:
  StackLessCo() : value_(kInit) {}

  inline bool IsComplete() const { return value_ == kExit; }

 private:
  friend class StackLessCoRef;
  int value_;
};

class StackLessCoRef {
 public:
  StackLessCoRef(StackLessCo& c) : value_(c.value_), modified_(false) {}
  StackLessCoRef(StackLessCo* c) : value_(c->value_), modified_(false) {}
  ~StackLessCoRef() {
    if (!modified_) value_ = -1;
  }
  operator int() const { return value_; }
  int& operator=(int v) {
    modified_ = true;
    return value_ = v;
  }

 private:
  int& value_;
  bool modified_;
};
}  // namespace libco

#define CO_BEGIN(c)                            \
  switch (libco::StackLessCoRef _co_value = c) \
  case libco::kExit:                           \
    if (_co_value) {                           \
      goto terminate_coroutine;                \
    terminate_coroutine:                       \
      _co_value = libco::kExit;                \
      goto bail_out_of_coroutine;              \
    bail_out_of_coroutine:                     \
      break;                                   \
    } else                                     \
    case libco::kInit:

#define CO_YIELD_IMPL(number)  \
  for (_co_value = (number);;) \
    if (_co_value == 0) {      \
      case (number):;          \
        break;                 \
    } else                     \
      return

#define CO_YIELD CO_YIELD_IMPL(__LINE__)
