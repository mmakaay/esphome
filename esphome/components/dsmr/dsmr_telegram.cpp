#include "esphome/core/log.h"
#include "dsmr_telegram.h"

namespace esphome {
namespace dsmr {

static const char *const TAG = "dsmr";

DsmrTelegram::DsmrTelegram(const char* type) : type_(type) {}

void DsmrTelegram::dump_telegram_config() {
    ESP_LOGCONFIG(TAG, "  Telegram type: %s", type_);
}

void DsmrTelegram::reset() {
  this->data_.clear();
  this->bytes_read_ = 0; // is data_.size()
  this->state_ = TELEGRAM_FIND_HEADER;
  this->is_reading_ = false;
  this->is_complete_ = false;
}

bool DsmrTelegram::is_reading() {
  return this->is_reading_;
}

bool DsmrTelegram::is_complete() {
  return this->is_complete_;
}

DsmrPlainTextTelegram::DsmrPlainTextTelegram() : PlainTextTelegram("Plain text") {}

DsmrPlainTextTelegram::add(const char byte) {
  switch (this->state_) {
    case TELEGRAM_FIND_HEADER:
      if (byte == '/') {
        this->reset();
        this->is_reading_ = true;
        this->state_ = TELEGRAM_FIND_FOOTER;
      }
    case TELEGRAM_FIND_FOOTER:
      if (byte == '!') {
        this->data_.push_back(byte);  
      }
      HIER
  }

  if (this->is_reading_) {
    this->data_.push_back(byte);  
  }
}

}  // namespace dsmr
}  // namespace esphome
