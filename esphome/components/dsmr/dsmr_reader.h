#pragma once

#include "dsmr_input.h"
#include "dsmr_telegram.h"

namespace esphome {
namespace dsmr {

class DsmrReader {
 // DsmrReader reads data from the DsmrInput and passes it on to the
 // DsmrTelegram, while keeping an eye on read timeouts.
 public:
  explicit DsmrReader(DsmrInput *input, DsmrTelegram *telegram);

  // Set the DsmrTelegram instance to use. TODO Do we really need this?
  // The only reason so far for setting it, is when the encryption key for
  // encrypted DSMR changes. But that can be handled by a set encryption key
  // method on the instance. The only reason that would remain is when
  // starting out with unencrypted DSMR and then setting the encryption key,
  // but I'm not sure if that's a useful use case. Doesn't make sense to me.
  void set_telegram(DsmrTelegram *telegram) { this->telegram_ = telegram; telegram->reset(); }

  // Set the timeout on incoming data while reading a telegram. When no new
  // data arrive within the given timeout, the device will consider the
  // current telegram a loss and starts looking for the header of the next
  // telegram.
  void set_receive_timeout(uint32_t timeout) { this->receive_timeout_ = timeout; }

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

  uint32_t receive_timeout_;
  size_t max_telegram_len_;
  bool receive_timeout_reached_();
  uint32_t last_receive_time_{0};
};

}  // namespace dsmr
}  // namespace esphome
