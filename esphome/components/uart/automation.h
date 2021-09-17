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
    parent->add_data_callback([this](UARTDirection direction, uint8_t byte) {
      if (this->pre_trigger_constraint_met_(direction)) { this->fire_trigger_(); }
      this->store_byte_(direction, byte);
      if (this->post_trigger_constraint_met_()) { this->fire_trigger_(); }
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

  bool pre_trigger_constraint_met_(UARTDirection direction) {
    // When debugging traffic in both the RX and TX direction, and a change of
    // direcion in traffic is detected, then fire this trigger when data are
    // available for the previous direction.
    if (this->bytes_.size() == 0) { return false; }
    if (this->for_direction_ != UART_DIRECTION_BOTH) { return false; }
    if (this->last_direction_ == direction) { return false; }
    return true;
  }

  void store_byte_(UARTDirection direction, uint8_t byte) {
    // Only store the new byte if it matches the direction for this object.
    if (this->for_direction_ == UART_DIRECTION_BOTH || this->for_direction_ == direction) {
      this->bytes_.push_back(byte);
      this->last_direction_ = direction;
    }
  }

  bool post_trigger_constraint_met_() {
    if (this->bytes_.size() == 0) { return false; }
    if (this->after_bytes_ > 0 && this->bytes_.size() >= this->after_bytes_) { return true; }
    // broken if (this->after_newline_ && *(bytes_.end()) == '\n') { return true; }
    // TODO delimiter
    // TODO timeout
    return false;
  }

  void fire_trigger_() {
    // DEBUG TODO temp for development purposes.
    if (this->last_direction_ == UART_DIRECTION_TX) {
      ESP_LOGE("DEBUG", ">>> %s", (std::string(this->bytes_.begin(), this->bytes_.end())).c_str());
    } else {
      ESP_LOGE("DEBUG", "<<< %s", (std::string(this->bytes_.begin(), this->bytes_.end())).c_str());
    }

    trigger(this->last_direction_, this->bytes_);
    this->bytes_.clear();
  }
};
#endif

}  // namespace uart
}  // namespace esphome
