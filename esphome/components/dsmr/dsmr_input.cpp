#include "dsmr_input.h"

namespace esphome {
namespace dsmr {

static const char *const TAG = "dsmr_input";

bool DsmrUARTInput::available() {
  return uart_->available();
}

const char DsmrUARTInput::read() {
  return uart_->read();
}

}  // namespace dsmr
}  // namespace esphome
