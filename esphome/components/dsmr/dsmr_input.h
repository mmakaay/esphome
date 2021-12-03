#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace dsmr {

class DsmrInput {
    /// Check if at least one byte is available on the input.
    virtual bool available() = 0;

    /// Read one byte from the input.
    virtual const char read() = 0;
};

class DsmrUARTInput : public DsmrInput {
 public:
  DsmrUARTInput(uart::UARTComponent *uart) {
    this->uart_ = new uart::UARTDevice(uart);
  }

  bool available() override;
  const char read() override;

 protected:
  uart::UARTDevice *uart_;
};

}  // namespace dsmr
}  // namespace esphome

