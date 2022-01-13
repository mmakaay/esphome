#include "esphome/core/log.h"
#include "plain_text_telegram.h"
#include "dsmr_telegram.h"
#include "dsmr.h"

namespace esphome {
namespace dsmr {

PlainTextTelegram::PlainTextTelegram() : DsmrTelegram("Plain text") {
  this->reset();
}

void PlainTextTelegram::reset() {
  DsmrTelegram::reset();
  this->crc_.clear();
  this->is_reading_ = false;
  this->state_ = PlainTextTelegramState::FindHeader;
}

void PlainTextTelegram::add(const char byte) {
  // If a new header is found, while no complete telegram has been read yet,
  // then reset the telegram data to start reading the new telegram.
  // Apparently some data got lost and it's of no use to keep reading like
  // nothing happened.
  // If a complete telegram has been read though, then wait for it to be
  // processed. After processing, reset() will be called to start reading
  // the next telegram.
  if (byte == '/' && this->state_ != PlainTextTelegramState::Complete) {
    this->reset();
  }

  // Read a telegram from the input stream.
  switch (this->state_) {
    case PlainTextTelegramState::FindHeader:
      if (byte == '/') {
        this->is_reading_ = true;
        this->state_ = PlainTextTelegramState::FindFooter;
      }
      break;
    case PlainTextTelegramState::FindFooter:
      if (byte == '!') {
        this->state_ = PlainTextTelegramState::ReadCRC;
      }
      break;
    case PlainTextTelegramState::ReadCRC:
      if (byte == '\n') {
          this->state_ = PlainTextTelegramState::Complete;
      } else {
          this->crc_.push_back(byte);
      }
      break;
    case PlainTextTelegramState::Complete:
      this->is_reading_ = false;     
      break;
  }

  if (this->is_reading_) {
    this->data_.push_back(byte);
  }
}

bool PlainTextTelegram::is_complete() {
  return this->state_ == PlainTextTelegramState::Complete;
}

}  // namespace dsmr
}  // namespace esphome
