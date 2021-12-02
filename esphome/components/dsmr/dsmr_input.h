#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
//#include "esphome/core/log.h"
//#include "esphome/core/defines.h"

namespace esphome {
namespace dsmr {

class DsmrInput {
    /// Check if at least one byte is available on the input.
    virtual bool available();

    /// Read one byte from the input.
    virtual const char read();
}

class DsmrUARTInput : public DsmrInput {
 public:
  DsmrInput(uart::UARTComponent *uart) : uart_(uart) {}

  void set_receive_timeout(uint32_t timeout) { this->receive_timeout_ = timeout; }

  void dump_config() override;

  /// Check if at least one byte is available on the input.
  bool available();

  /// Read one byte from the input.
  const char read();

 protected:
  uart_::UARTDevice *uart;

  /// Wait for UART data to become available within the read timeout.
  ///
  /// The smart meter might provide data in chunks, causing available() to
  /// return 0. When we're already reading a telegram, then we don't return
  /// right away (to handle further data in an upcoming loop) but wait a
  /// little while using this method to see if more data are incoming.
  /// By not returning, we prevent other components from taking so much
  /// time that the UART RX buffer overflows and bytes of the telegram get
  /// lost in the process.
  bool available_within_timeout_();
  uint32_t receive_timeout_;
  bool receive_timeout_reached_();
};
}  // namespace dsmr
}  // namespace esphome

