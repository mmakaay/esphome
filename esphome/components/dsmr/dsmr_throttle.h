#pragma once

#include "dsmr_input.h"

namespace esphome {
namespace dsmr {

class DsmrThrottle : public Component {
 public:
  explicit DsmrThrottle(DsmrInput *input);

  void set_request_pin(GPIOPin *request_pin) { this->request_pin_ = request_pin; }
  void set_request_interval(uint32_t interval) { this->request_interval_ = interval; }

  void setup() override;
  void dump_throttle_config();

  bool ready_to_read();
  void wait_for_next();

 protected:
  DsmrInput *input_;
  uint32_t request_interval_{0};
  bool request_interval_reached_();
  GPIOPin *request_pin_{nullptr};
  uint32_t last_request_time_{0};
  bool requesting_data_{false};
  void start_requesting_data_();
};

}  // namespace dsmr
}  // namespace esphome
