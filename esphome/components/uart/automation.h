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
  void set_after_bytes(size_t size) { this->after_bytes_ = size; }
  void set_after_timeout(uint32_t timeout) { this->after_timeout_ = timeout; }
  void set_after_newline(bool enabled) { this->after_newline_ = enabled; }
  // TODO void set_after_delimiter(..) { ... }
  void set_before_transmit(bool enabled) { this->before_transmit_ = enabled; }
  void set_before_receive(bool enabled) { this->before_receive_ = enabled; }

 protected:
  std::vector<uint8_t> bytes_{};
  size_t after_bytes_;
  uint32_t after_timeout_;
  bool after_newline_;
  bool before_transmit_;
  bool before_receive_;
  UARTDirection for_direction_;

  void setup_(UARTComponent *parent, UARTDirection direction) {
    for_direction_ = direction;
    parent->add_data_callback([this, parent](UARTDirection direction, uint8_t byte) {
      add_(direction, byte);
    });
  }

  void add_(UARTDirection direction, uint8_t byte) {
    if (this->for_direction_ == direction) {
      bytes_.push_back(byte);
    }
    if (this->do_trigger_(direction, byte)) {
      ESP_LOGE("DEBUG", ">>> %s", (std::string(bytes_.begin(), bytes_.end())).c_str());
      trigger(direction, bytes_);
      bytes_.clear();
    }
  }

  bool do_trigger_(UARTDirection  direction, uint8_t byte) {
      if (bytes_.size() == 0) { return false; }
      if (this->after_bytes_ > 0 && bytes_.size() >= this->after_bytes_) { return true; }
      if (this->after_newline_ && byte == '\n') { return true; }
      if (this->before_transmit_ && direction == UART_RECEIVE) { return true; }
      if (this->before_receive_ && direction == UART_TRANSMIT) { return true; }
      // TODO timeout
      return false;
  }
};

class UARTReceiveTrigger : public UARTDataTrigger {
 public:
  explicit UARTReceiveTrigger(UARTComponent *parent) {
    this->setup_(parent, UART_RECEIVE);
  }
};

class UARTTransmitTrigger : public UARTDataTrigger {
 public:
  explicit UARTTransmitTrigger(UARTComponent *parent) {
    this->setup_(parent, UART_TRANSMIT);
  }
};
#endif

}  // namespace uart
}  // namespace esphome
