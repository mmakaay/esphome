#pragma once

#include <vector>
#include "esphome/core/defines.h"
#include "esphome/core/automation.h"
#include "uart.h"

namespace esphome {
namespace uart {

template<typename... Ts> class UARTWriteAction : public Action<Ts...>, public Parented<UARTComponent> {
 public:
  void set_data_template(std::function<std::vector<uint8_t>(Ts...)> func) {
    this->data_func_ = func;
    this->static_ = false;
  }
  void set_data_static(const std::vector<uint8_t> &data) {
    this->data_static_ = data;
    this->static_ = true;
  }

  void play(Ts... x) override {
    if (this->static_) {
      this->parent_->write_array(this->data_static_);
    } else {
      auto val = this->data_func_(x...);
      this->parent_->write_array(val);
    }
  }

 protected:
  bool static_{false};
  std::function<std::vector<uint8_t>(Ts...)> data_func_{};
  std::vector<uint8_t> data_static_{};
};

#ifdef USE_UART_DATA_TRIGGER
class UARTDataTrigger : public Trigger<UARTDirection, std::vector<uint8_t>> {
 public:
  explicit UARTDataTrigger(UARTComponent *parent) {
    parent->add_data_callback([this, parent](UARTDirection direction, uint8_t byte) {
      add_(direction, byte);
    });
  }
  void set_direction(UARTDirection direction) { this->for_direction_ = direction; }
  void set_after_bytes(size_t size) { this->after_bytes_ = size; }
  void set_after_timeout(uint32_t timeout) { this->after_timeout_ = timeout; }
  void set_after_newline(bool enabled) { this->after_newline_ = enabled; }
  // TODO void set_after_delimiter(..) { ... }

 protected:
  UARTDirection for_direction_;
  UARTDirection last_direction_;
  std::vector<uint8_t> bytes_{};
  size_t after_bytes_;
  uint32_t after_timeout_;
  bool after_newline_;

  void add_(UARTDirection direction, uint8_t byte) {
    // When debugging traffic in both the RX and TX direction, and a change of
    // direcion in traffic is detected, then fire this trigger when data are
    // available for the previous direction.
    if (for_direction_ == UART_DIRECTION_BOTH && last_direction_ != direction && bytes_.size()) {
      if (last_direction_ == UART_DIRECTION_TX) {
        ESP_LOGE("DEBUG", ">>> %s", (std::string(bytes_.begin(), bytes_.end())).c_str());
      } else {
        ESP_LOGE("DEBUG", "<<< %s", (std::string(bytes_.begin(), bytes_.end())).c_str());
      }
      trigger(last_direction_, bytes_);
      bytes_.clear();
    }
    // Store the new byte if it matches the direction for this object.
    if (this->for_direction_ == UART_DIRECTION_BOTH || this->for_direction_ == direction) {
      bytes_.push_back(byte);
      last_direction_ = direction;
    }
    // When a constaint for triggering is met, then fire this trigger.
    if (this->do_trigger_(direction, byte)) {
      if (last_direction_ == UART_DIRECTION_TX) {
        ESP_LOGE("DEBUG", ">>> %s", (std::string(bytes_.begin(), bytes_.end())).c_str());
      } else {
        ESP_LOGE("DEBUG", "<<< %s", (std::string(bytes_.begin(), bytes_.end())).c_str());
      }
      trigger(last_direction_, bytes_);
      bytes_.clear();
    }
  }

  bool do_trigger_(UARTDirection  direction, uint8_t byte) {
      if (bytes_.size() == 0) { return false; }
      if (this->after_bytes_ > 0 && bytes_.size() >= this->after_bytes_) { return true; }
      if (this->after_newline_ && byte == '\n') { return true; }
      // TODO delimiter
      // TODO timeout
      return false;
  }
};
#endif

}  // namespace uart
}  // namespace esphome
