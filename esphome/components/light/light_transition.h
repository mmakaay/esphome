#pragma once

#include "esphome/core/helpers.h"
#include "light_color_values.h"

namespace esphome {
namespace light {

class LightState;

/// Base class for light transitions.
class LightTransition {
 public:
  explicit LightTransition(std::string name) : name_(std::move(name)) {}

  void setup(LightState *state, const LightColorValues &start_values, const LightColorValues &target_values,
             uint32_t length) {
    this->state_ = state;
    this->start_time_ = millis();
    this->length_ = length;
    this->start_values_ = start_values;
    this->target_values_ = target_values;
    this->start();
  }

  /// Indicates whether this transformation is completed and should be stopped.
  virtual bool is_completed() { return this->get_progress_() >= 1.0f; }

  /// This will be called before the transition is started.
  virtual void start() {}

  /// This will be called while the transition is active to apply the transition to the light. Can either write to the
  /// light directly, or return LightColorValues that will be applied.
  virtual optional<LightColorValues> apply() = 0;

  /// This will be called after transition is completed.
  virtual void finish() {}

  /// This will be called if the transition is aborted before it is completed.
  virtual void abort() {}

  const LightColorValues &get_start_values() const { return this->start_values_; }

  const LightColorValues &get_target_values() const { return this->target_values_; }

  const std::string &get_name() { return this->name_; }

 protected:
  /// The progress of this transition, on a scale of 0 to 1.
  float get_progress_() { return clamp((millis() - this->start_time_) / float(this->length_), 0.0f, 1.0f); }

  std::string name_;

  uint32_t start_time_;
  uint32_t length_;
  LightColorValues start_values_;
  LightColorValues target_values_;
  LightState *state_{nullptr};
};

}  // namespace light
}  // namespace esphome