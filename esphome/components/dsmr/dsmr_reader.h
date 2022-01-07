#pragma once

#include "dsmr_input.h"
#include "dsmr_telegram.h"

namespace esphome {
namespace dsmr {

class DsmrReader {
 public:
  explicit DsmrReader(DsmrInput *input);

  void set_receive_timeout(uint32_t timeout) { this->receive_timeout_ = timeout; }
  void set_telegram(DsmrTelegram *telegram) { this->telegram_ = telegram; }
  void set_max_telegram_length(size_t length) { this->max_telegram_len_ = length; } // WEG

  void dump_reader_config();

  bool available();
  const char read();
  size_t bytes_read();

  // In a further iteration, I want to get rid of these.
  // Only implementing them now to have an in-between refactoring step.
  void pop_byte() { this->bytes_read_--; }
  void set_bytes_read(size_t bytes) { this->bytes_read_ = bytes; }

  void reset();
  void set_header_found();
  bool header_found();
  void set_footer_found();
  bool footer_found();

 protected:
  DsmrInput *input_;
  DsmrTelegram *telegram_;

  size_t bytes_read_{0}; // WEG

  bool header_found_{false}; // WEG
  bool footer_found_{false}; // WEG

  /// Wait for UART data to become available within the read timeout.
  ///
  /// The smart meter might provide data in chunks, causing available() to
  /// return 0. When we're already reading a telegram, then we don't return
  /// right away (to handle further data in an upcoming loop) but wait a
  /// little while using this method to see if more data are incoming.
  /// By not returning, we prevent other components from taking so much
  /// time that the UART RX buffer overflows and bytes of the telegram get
  /// lost in the process.
  uint32_t receive_timeout_;
  size_t max_telegram_len_;
  bool receive_timeout_reached_();
  uint32_t last_receive_time_{0};
};

}  // namespace dsmr
}  // namespace esphome
