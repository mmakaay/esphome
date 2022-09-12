#pragma once

#include "esphome/components/dsmr/dsmr_input.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace dsmr {

class DsmrUARTInput : public DsmrInput {
 // DsmrUARTInput implements the DsmrInput interface for a smart meter
 // that provides its data through a UART serial interface.
 public:
  explicit DsmrUARTInput(uart::UARTComponent *uart) {
    this->uart_ = new uart::UARTDevice(uart);
    this->buffer_size_ = uart->get_rx_buffer_size();
  }

  bool available() override {
    return this->uart_->available();
  }

  const char read() override {
    return this->uart_->read();
  }

  bool can_buffer(size_t bytes) override {
    return this->buffer_size_ >= bytes; 
  }

 protected:
  size_t buffer_size_;
  uart::UARTDevice *uart_;
};

}  // namespace dsmr
}  // namespace esphome

