#pragma once

#include "dsmr_input.h"

namespace esphome {
namespace dsmr {

class DsmrThrottle : public Component {
 // The DsmrThrottle class provides functionality to limit how often telegrams
 // are read from a smart meter. Two mechanisms are provided for this:
 // 1. Recommended: using a hardware GPIO pin to signal the smart meter that
 //    data can be sent.
 // 2. Draining input data from the smart meter, until the time at which new
 //    data are to be read.
 public:
  explicit DsmrThrottle(DsmrInput *input);

  // Optionally, configure the GPIO pin that can be used to signal the smart
  // meter (by setting the pin output to HIGH) that it can send new
  // telegram data.
  void set_request_pin(GPIOPin *request_pin) { this->request_pin_ = request_pin; }

  // Optionally, set the minimum time between two telegram readings.
  // By default, the request interval is set to 0, meaning that the pace at
  // which the smart meter sends its data determines the update frequency.
  void set_request_interval(uint32_t interval) { this->request_interval_ = interval; }

  void setup() override;

  void dump_throttle_config();

  // Check if a request interval has passed. Until then, throttle the smart
  // meter data.
  bool ready_to_read();

  // Tell the throttle that data have been read and that it must now wait
  // for the next request interval to pass.
  void wait_for_next();

 protected:
  DsmrInput *input_;
  uint32_t request_interval_{0};
  bool request_interval_reached_();
  GPIOPin *request_pin_{nullptr};
  uint32_t last_request_time_{0};
  bool requesting_data_{false};
  void start_requesting_data_();
  void stop_requesting_data_();
};

}  // namespace dsmr
}  // namespace esphome
