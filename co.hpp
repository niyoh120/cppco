#pragma once

#include <ucontext.h>

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace co {

typedef std::shared_ptr<void> AnyPtr;
constexpr auto k_default_stack_size = 8 * 1024;

enum class State {
  Idle,
  Running,
  Finish,
};

class Generator {
 public:
  static AnyPtr yield(const AnyPtr &p = nullptr) {
    return current_->yield_to(p);
  };

  static AnyPtr yield_from(const std::shared_ptr<Generator> &g) {
    if (current_ == &root_) {
      return nullptr;
    }
    current_->value_ = nullptr;
    if (g->state() == State::Idle) {
      current_->value_ = yield(g->next());
    }
    while (g->state() != State::Finish) {
      current_->value_ = yield(g->send(current_->value_));
    }
    return current_->value_;
  }

  Generator(const Generator &) = delete;
  Generator &operator=(const Generator &) = delete;

  explicit Generator(int stack_size) : stack_(stack_size) {
    int ret = ::getcontext(&ctx_);
    assert(ret == 0);
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = &stack_[0];
    ctx_.uc_stack.ss_size = stack_.size();
    ::makecontext(&ctx_, reinterpret_cast<void (*)(void)>(&run), 1, this);
  }

  template <typename F, typename = typename std::enable_if<std::is_void<typename std::result_of<F()>::type>::value>::type, typename Dummy = void>
  Generator(int stack_size, F f) : Generator(stack_size) {
    f_ = std::move(f);
  }

  template <typename F, typename = typename std::enable_if<!std::is_void<typename std::result_of<F()>::type>::value>::type>
  Generator(int stack_size, F f) : Generator(stack_size) {
    typedef typename std::result_of<F()>::type R;

    f_ = [this, f]() {
      value_ = std::make_shared<R>(std::move(f()));
    };
  }

  State state() {
    return state_;
  }

  AnyPtr send(const AnyPtr &p = nullptr) {
    if (state_ != State::Running) {
      return nullptr;
    }
    return send_to(p);
  };

  AnyPtr next() {
    if (state_ == State::Finish) {
      return nullptr;
    }
    return send_to();
  };

 private:
  static void run(Generator *g) {
    assert(g);
    assert(current_ == g);
    g->state_ = State::Running;
    g->f_();
    g->state_ = State::Finish;
    yield(g->value_);
  };

  Generator() : state_(State::Running) {
    current_ = this;
  };

  AnyPtr send_to(const AnyPtr &p = nullptr) {
    assert(current_ != this);
    parrent_ = current_;
    value_ = p;
    current_ = this;
    int ret = ::swapcontext(&parrent_->ctx_, &ctx_);
    assert(ret == 0);
    return value_;
  }

  AnyPtr yield_to(const AnyPtr &p = nullptr) {
    assert(current_ == this);
    assert(parrent_ != nullptr);
    value_ = p;
    current_ = parrent_;
    int ret = ::swapcontext(&ctx_, &parrent_->ctx_);
    assert(ret == 0);
    return value_;
  }

 private:
  static Generator *current_;
  static Generator root_;
  ::ucontext_t ctx_;
  std::vector<char> stack_;
  State state_ = State::Idle;
  AnyPtr value_ = nullptr;
  std::function<void()> f_;
  Generator *parrent_ = nullptr;
};

inline AnyPtr yield(const AnyPtr &p = nullptr) {
  return Generator::yield(p);
}

template <typename F>
std::shared_ptr<Generator> async(F &&f) {
  return std::make_shared<Generator>(k_default_stack_size, std::forward<F>(f));
};

inline AnyPtr await(const std::shared_ptr<Generator> &g) {
  return Generator::yield_from(g);
}

}  // namespace co
