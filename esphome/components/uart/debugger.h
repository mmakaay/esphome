#pragma once
#ifdef USE_UART_DEBUGGER

#include <vector>
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "uart.h"

namespace esphome {
namespace uart {

static const char *const TAG = "uart_debugger";

class UARTDataTrigger : public Trigger<UARTDirection, std::vector<uint8_t>> {
 public:
  explicit UARTDataTrigger(UARTComponent *parent) {
    parent->add_data_callback([this](UARTDirection direction, uint8_t byte) {
      if (!is_my_direction_(direction)) { return; }
      if (is_recursive_(direction)) { return; }
      if (this->direction_changed_(direction) && this->bytes_.size()) {
        this->fire_trigger_(this->last_direction_);
      }
      this->store_byte_(direction, byte);
      if (this->meets_trigger_constraints_()) { this->fire_trigger_(direction); }
    });
  }

  void set_direction(UARTDirection direction) { this->for_direction_ = direction; }
  void set_after_bytes(size_t size) { this->after_bytes_ = size; }
  void set_after_timeout(uint32_t timeout) { this->after_timeout_ = timeout; }
  void set_after_newline(bool enabled) { this->after_newline_ = enabled; }

 protected:
  UARTDirection for_direction_;
  UARTDirection last_direction_;
  std::vector<uint8_t> bytes_{};
  bool triggering_{false};
  bool recurse_warning_issued_{false};

  // Properties that define when to trigger a bytes dump.
  size_t after_bytes_;
  uint32_t after_timeout_;
  bool after_newline_;

  bool is_my_direction_(UARTDirection direction) {
    if (this->for_direction_ == UART_DIRECTION_BOTH) { return true; }
    if (this->for_direction_ == direction) { return true; }
    return false;
  }

  bool is_recursive_(UARTDirection direction) {
    if (this->triggering_) {
      if (!recurse_warning_issued_) {
        ESP_LOGW(TAG, "Data %s while UART debugger is triggering its actions",
                 direction == UART_DIRECTION_RX ? "received" : "transmitted");
        ESP_LOGW(TAG, "Ignoring to prevent recursive debugging loops");
        recurse_warning_issued_ = true;
      }
      return true;
    }
	return false;
  }

  bool direction_changed_(UARTDirection direction) {
    if (this->for_direction_ != UART_DIRECTION_BOTH) { return false; }
    if (this->last_direction_ == direction) { return false; }
    return true;
  }

  void store_byte_(UARTDirection direction, uint8_t byte) {
    this->bytes_.push_back(byte);
    this->last_direction_ = direction;
  }

  bool meets_trigger_constraints_() {
    if (this->bytes_.size() == 0) { return false; }
    if (this->after_bytes_ > 0 && this->bytes_.size() >= this->after_bytes_) { return true; }
    return false;
  }

  void fire_trigger_(UARTDirection direction) {
    this->triggering_ = true;
    trigger(direction, this->bytes_);
    this->bytes_.clear();
    this->triggering_ = false;
  }
};

class UARTDebuggerRXSink : public Component, public UARTDevice {
 public:
  UARTDebuggerRXSink(UARTComponent *parent) : UARTDevice(parent) {}
  void loop() override {
    int count = 100;
    while (this->available() && count--) {
      this->read();
    }
  }
};

}  // namespace uart
}  // namespace esphome
#endif
