#include <deque>
#include <sys/time.h>
#include "dsmr_input.h"

namespace esphome {
namespace dsmr {

size_t get_time_in_ms() {
  time_t now = time(nullptr);
  time_t mnow = now * 1000;
  /////////////////////std::cout << "T=" << mnow;
  return mnow;
}

struct DsmrStubInputStep {
  size_t after_ms = 0;
  std::deque<char> chars = {};
};

class DsmrStubInput : public DsmrInput {
 public:
  explicit DsmrStubInput() {}
  
  DsmrStubInput *add(char c, size_t after_ms = 0) {
    auto chars = std::deque<char>();
    struct DsmrStubInputStep step;
    step.after_ms = after_ms;
    step.chars.push_back(c);
    this->steps_.push_back(step);
    return this;
  }

  bool available() override {
    // No more steps available?
    if (this->steps_.empty()) {
      return false;
    }

    // Initialize the step timer.
    if (this->step_start_ms_ == 0) {
        this->step_start_ms_ = get_time_in_ms();
    }

    // Check if the wait time for the first step in line has passed.
    if (!this->in_step_) {
      auto step = this->steps_[0];
      auto time_in_this_step = get_time_in_ms() - this->step_start_ms_;
      if (time_in_this_step >= step.after_ms) {
        // If yes, then we can start providing chars from this step.
        this->step_start_ms_ = get_time_in_ms();
        this->current_step_ = step;
        this->in_step_ = true;
        this->steps_.pop_front();
      }
    }

    // Next step not up yet?
    if (!this->in_step_) {
      return false;
    }

    // No more chars left in the current step? Then recurse into
    // the available() function to see if the next step already
    // provides a char.
    if (this->in_step_ && this->current_step_.chars.empty()) {
      this->in_step_ = false;
      return this->available();
    }

    // Yes, we've got at least one char available for the caller.
    return true;
  }
  
  const char read() override {
    char c = this->current_step_.chars[0];
    this->current_step_.chars.pop_front();
    return c;
  }
  
  bool can_buffer(size_t bytes) override {
    return false;
  }
  
 protected:
  std::deque<DsmrStubInputStep> steps_{};
  struct DsmrStubInputStep current_step_{};
  bool in_step_{false};
  size_t step_start_ms_{0};
};

}  // namespace dsmr
}  // namespace esphome