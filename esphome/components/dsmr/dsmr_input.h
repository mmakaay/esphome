#pragma once

#include "esphome/components/uart/uart.h"

namespace esphome {
namespace dsmr {

class DsmrInput {
 public:
  /// Check if at least one byte is available on the input.
  virtual bool available() = 0;

  /// Read one byte from the input.
  virtual const char read() = 0;

  /// Check if the input can buffer the provided number of bytes.
  virtual bool can_buffer(size_t bytes) = 0;
};

class DsmrUARTInput : public DsmrInput {
 public:
  explicit DsmrUARTInput(uart::UARTComponent *uart);

  bool available() override;
  const char read() override;
  bool can_buffer(size_t bytes) override;

 protected:
  size_t buffer_size_;
  uart::UARTDevice *uart_;
};

}  // namespace dsmr
}  // namespace esphome

