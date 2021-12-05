#include "dsmr_input.h"

namespace esphome {
namespace dsmr {

static const char *const TAG = "dsmr";

DsmrUARTInput::DsmrUARTInput(uart::UARTComponent *uart) {
  this->uart_ = new uart::UARTDevice(uart);
  this->buffer_size_ = uart->get_rx_buffer_size();
}

bool DsmrUARTInput::available() {
  return this->uart_->available();
}

const char DsmrUARTInput::read() {
  return this->uart_->read();
}

bool DsmrUARTInput::can_buffer(size_t bytes) {
  return this->buffer_size_ >= bytes; 
}

}  // namespace dsmr
}  // namespace esphome
