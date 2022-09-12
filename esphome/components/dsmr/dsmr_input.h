#pragma once

namespace esphome {
namespace dsmr {

class DsmrInput {
 // DsmrInput provides the interface that the DSMR component uses to receive
 // bytes of input (telegrams) from a smart meter.
 public:
  /// Check if at least one byte is available on the input.
  virtual bool available() = 0;

  /// Read one byte from the input.
  virtual const char read() = 0;

  /// Check if the input can buffer the provided number of bytes.
  virtual bool can_buffer(size_t bytes) = 0;

  /// Drain all input that is currently available.
  void drain() {
    while (available()) {
      read();
    }
  }
};

}  // namespace dsmr
}  // namespace esphome

