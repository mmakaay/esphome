#include "esphome/core/log.h"

#include "dsmr_input.h"
#include "dsmr_reader.h"

namespace esphome {
namespace dsmr {

static const char *const TAG = "dsmr";

DsmrReader::DsmrReader(DsmrInput *input) : input_(input) {}

void DsmrReader::dump_reader_config() {
  ESP_LOGCONFIG(TAG, "  Receive timeout: %.1fs", this->receive_timeout_ / 1e3f);
  this->telegram_->log_telegram_config();
}

void DsmrReader::reset() {
  this->last_receive_time_ = 0; // WEG ?
  this->telegram_->reset();
}

void DsmrReader::set_header_found() {
  this->header_found_ = true;
}

bool DsmrReader::header_found() {
  return this->header_found_;
}

void DsmrReader::set_footer_found() {
  this->footer_found_ = true;
}

bool DsmrReader::footer_found() {
  return this->footer_found_;
}

bool DsmrReader::available() {
  // Check for excessive telegram input data.
  if (this->bytes_read() >= this->max_telegram_len_) {
    ESP_LOGE(TAG, "Error: telegram larger than max expected size (%d bytes)", this->max_telegram_len_);
    this->reset();
  }

  // Data are available for reading on the UART bus?
  // Then we can start reading right away.
  if (this->input_->available()) {
    this->last_receive_time_ = millis();
    return true;
  }
  // When we're not in the process of reading a telegram, then there is
  // no need to actively wait for new data to come in.
  if (!this->header_found_) {
    return false;
  }
  // A telegram is being read. The smart meter might not deliver a telegram
  // in one go, but instead send it in chunks with small pauses in between.
  // When the UART RX buffer cannot hold a full telegram, then make sure
  // that the UART read buffer does not overflow while other components
  // perform their work in their loop. Do this by not returning control to
  // the main loop, until the read timeout is reached.
  if (!this->input_->can_buffer(this->max_telegram_len_)) {
    while (!this->receive_timeout_reached_()) {
      delay(5);
      if (this->input_->available()) {
        this->last_receive_time_ = millis();
        return true;
      }
    }
  }
  // No new data has come in during the read timeout? Then stop reading the
  // telegram and start waiting for the next one to arrive.
  if (this->receive_timeout_reached_()) {
    ESP_LOGW(TAG, "Timeout while reading data for telegram");
    this->reset();
  }

  return false;
}

bool DsmrReader::receive_timeout_reached_() {
  return millis() - this->last_receive_time_ > this->receive_timeout_;
}

const char DsmrReader::read() {
  if (this->header_found_) {
    this->bytes_read_++;
  }
  return this->input_->read();
}

size_t DsmrReader::bytes_read() {
  return this->bytes_read_;
}

}  // namespace dsmr
}  // namespace esphome
