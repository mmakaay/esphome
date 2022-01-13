#include "esphome/core/log.h"
#include "dsmr.h"
#include "dsmr_input.h"
#include "dsmr_throttle.h"

namespace esphome {
namespace dsmr {

DsmrThrottle::DsmrThrottle(DsmrInput *input) : input_(input) {}

void DsmrThrottle::setup() {
  if (this->request_pin_ != nullptr) {
    this->request_pin_->setup();
  }
}

void DsmrThrottle::dump_throttle_config() {
  if (this->request_pin_ != nullptr) {
    LOG_PIN("  Request Pin: ", this->request_pin_);
  }
  if (this->request_interval_ > 0) {
    ESP_LOGCONFIG(TAG, "  Request Interval: %.1fs", this->request_interval_ / 1e3f);
  }
}

bool DsmrThrottle::ready_to_read() {
  // When using a request pin, then wait for the next request interval.
  if (this->request_pin_ != nullptr) {
    if (!this->requesting_data_ && this->request_interval_reached_()) {
      this->start_requesting_data_();
    }
  }
  // Otherwise, sink serial data until next request interval.
  else {
    if (this->request_interval_reached_()) {
      this->start_requesting_data_();
    }
    if (!this->requesting_data_) {
      this->input_->drain();
    }
  }
  return this->requesting_data_;
}

bool DsmrThrottle::request_interval_reached_() {
  if (this->last_request_time_ == 0) {
    return true;
  }
  return millis() - this->last_request_time_ > this->request_interval_;
}

void DsmrThrottle::start_requesting_data_() {
  if (!this->requesting_data_) {
    if (this->request_pin_ != nullptr) {
      ESP_LOGV(TAG, "Start requesting data from P1 port");
      this->request_pin_->digital_write(true);
    } else {
      ESP_LOGV(TAG, "Start reading data from P1 port");
    }
    this->requesting_data_ = true;
    this->last_request_time_ = millis();
  }
}

void DsmrThrottle::wait_for_next() {
  this->stop_requesting_data_();
}

void DsmrThrottle::stop_requesting_data_() {
  if (this->requesting_data_) {
    if (this->request_pin_ != nullptr) {
      ESP_LOGV(TAG, "Stop requesting data from P1 port");
      this->request_pin_->digital_write(false);
    } else {
      ESP_LOGV(TAG, "Stop reading data from P1 port");
    }
    this->input_->drain();
    this->requesting_data_ = false;
  }
}

}  // namespace dsmr
}  // namespace esphome
