#ifdef USE_ARDUINO

#include "dsmr.h"
#include "esphome/core/log.h"

#include <AES.h>
#include <Crypto.h>
#include <GCM.h>

namespace esphome {
namespace dsmr {

static const char *const TAG = "dsmr";

void Dsmr::setup() {
  this->telegram_ = new char[this->max_telegram_len_];  // NOLINT
}

void Dsmr::loop() {
  if (this->throttle_->ready_to_read()) {
    if (this->decryption_key_.empty()) {
      this->receive_telegram_();
    } else {
      this->receive_encrypted_telegram_();
    }
  }
}

void Dsmr::receive_telegram_() {
  while (this->reader_->available()) {
    const char c = this->reader_->read();

    // Find a new telegram header, i.e. forward slash.
    if (c == '/') {
      ESP_LOGV(TAG, "Header of telegram found");
      this->reader_->reset();
      this->reader_->set_header_found();
    }
    if (!this->reader_->header_found())
      continue;

    // Check for buffer overflow.
    if (this->reader_->bytes_read() >= this->max_telegram_len_) {
      this->reader_->reset();
      ESP_LOGE(TAG, "Error: telegram larger than buffer (%d bytes)", this->max_telegram_len_);
      return;
    }

    // Some v2.2 or v3 meters will send a new value which starts with '('
    // in a new line, while the value belongs to the previous ObisId. For
    // proper parsing, remove these new line characters.
    if (c == '(') {
      while (true) {
        auto previous_char = this->telegram_[this->reader_->bytes_read() - 1];
        if (previous_char == '\n' || previous_char == '\r') {
          this->reader_->pop_byte();
        } else {
          break;
        }
      }
    }

    // Store the byte in the buffer.
    this->telegram_[this->reader_->bytes_read()] = c;

    // Check for a footer, i.e. exlamation mark, followed by a hex checksum.
    if (c == '!') {
      ESP_LOGV(TAG, "Footer of telegram found");
      this->reader_->set_footer_found();
      continue;
    }
    // Check for the end of the hex checksum, i.e. a newline.
    if (this->reader_->footer_found() && c == '\n') {
      // Parse the telegram and publish sensor values.
      this->parse_telegram();
      this->reader_->reset();
      return;
    }
  }
}

void Dsmr::receive_encrypted_telegram_() {
  while (this->reader_->available()) {
    const char c = this->reader_->read();

    // Find a new telegram start byte.
    if (!this->reader_->header_found()) {
      if ((uint8_t) c != 0xDB) {
        continue;
      }
      ESP_LOGV(TAG, "Start byte 0xDB of encrypted telegram found");
      this->reader_->reset();
      this->reader_->set_header_found();
      this->crypt_telegram_len_ = 0;
    }

    // Check for buffer overflow.
    if (this->reader_->bytes_read() >= this->max_telegram_len_) {
      this->reader_->reset();
      ESP_LOGE(TAG, "Error: encrypted telegram larger than buffer (%d bytes)", this->max_telegram_len_);
      return;
    }

    // Store the byte in the buffer.
    this->crypt_telegram_[this->reader_->bytes_read()] = c;

    // Read the length of the incoming encrypted telegram.
    if (this->crypt_telegram_len_ == 0 && this->reader_->bytes_read() > 20) {
      // Complete header + data bytes
      this->crypt_telegram_len_ = 13 + (this->crypt_telegram_[11] << 8 | this->crypt_telegram_[12]);
      ESP_LOGV(TAG, "Encrypted telegram length: %d bytes", this->crypt_telegram_len_);
    }

    // Check for the end of the encrypted telegram.
    if (this->crypt_telegram_len_ == 0 || this->reader_->bytes_read() != this->crypt_telegram_len_) {
      continue;
    }
    ESP_LOGV(TAG, "End of encrypted telegram found");

    // Decrypt the encrypted telegram.
    GCM<AES128> *gcmaes128{new GCM<AES128>()};
    gcmaes128->setKey(this->decryption_key_.data(), gcmaes128->keySize());
    // the iv is 8 bytes of the system title + 4 bytes frame counter
    // system title is at byte 2 and frame counter at byte 15
    for (int i = 10; i < 14; i++)
      this->crypt_telegram_[i] = this->crypt_telegram_[i + 4];
    constexpr uint16_t iv_size{12};
    gcmaes128->setIV(&this->crypt_telegram_[2], iv_size);
    gcmaes128->decrypt(reinterpret_cast<uint8_t *>(this->telegram_),
                       // the ciphertext start at byte 18
                       &this->crypt_telegram_[18],
                       // cipher size
                       this->reader_->bytes_read() - 17);
    delete gcmaes128;  // NOLINT(cppcoreguidelines-owning-memory)

    this->reader_->set_bytes_read(strnlen(this->telegram_, this->max_telegram_len_));
    ESP_LOGV(TAG, "Decrypted telegram size: %d bytes", this->reader_->bytes_read());
    ESP_LOGVV(TAG, "Decrypted telegram: %s", this->telegram_);

    // Parse the decrypted telegram and publish sensor values.
    this->parse_telegram();
    this->reader_->reset();
    return;
  }
}

bool Dsmr::parse_telegram() {
  MyData data;
  ESP_LOGV(TAG, "Trying to parse telegram");
  this->throttle_->wait_for_next();
  ::dsmr::ParseResult<void> res =
      ::dsmr::P1Parser::parse(&data, this->telegram_, this->reader_->bytes_read(), false,
                              this->crc_check_);  // Parse telegram according to data definition. Ignore unknown values.
  if (res.err) {
    // Parsing error, show it
    auto err_str = res.fullError(this->telegram_, this->telegram_ + this->reader_->bytes_read());
    ESP_LOGE(TAG, "%s", err_str.c_str());
    return false;
  } else {
    this->status_clear_warning();
    this->publish_sensors(data);
    return true;
  }
}

void Dsmr::dump_config() {
  ESP_LOGCONFIG(TAG, "DSMR:");
  ESP_LOGCONFIG(TAG, "  Max telegram length: %d", this->max_telegram_len_);
  this->throttle_->dump_throttle_config();
  this->reader_->dump_reader_config();

#define DSMR_LOG_SENSOR(s) LOG_SENSOR("  ", #s, this->s_##s##_);
  DSMR_SENSOR_LIST(DSMR_LOG_SENSOR, )

#define DSMR_LOG_TEXT_SENSOR(s) LOG_TEXT_SENSOR("  ", #s, this->s_##s##_);
  DSMR_TEXT_SENSOR_LIST(DSMR_LOG_TEXT_SENSOR, )
}

void Dsmr::set_decryption_key(const std::string &decryption_key) {
  if (decryption_key.length() == 0) {
    ESP_LOGI(TAG, "Disabling decryption");
    this->decryption_key_.clear();
    if (this->crypt_telegram_ != nullptr) {
      delete[] this->crypt_telegram_;
      this->crypt_telegram_ = nullptr;
    }
    return;
  }

  if (decryption_key.length() != 32) {
    ESP_LOGE(TAG, "Error, decryption key must be 32 character long");
    return;
  }
  this->decryption_key_.clear();

  ESP_LOGI(TAG, "Decryption key is set");
  // Verbose level prints decryption key
  ESP_LOGV(TAG, "Using decryption key: %s", decryption_key.c_str());

  char temp[3] = {0};
  for (int i = 0; i < 16; i++) {
    strncpy(temp, &(decryption_key.c_str()[i * 2]), 2);
    this->decryption_key_.push_back(std::strtoul(temp, nullptr, 16));
  }

  if (this->crypt_telegram_ == nullptr) {
    this->crypt_telegram_ = new uint8_t[this->max_telegram_len_];  // NOLINT
  }
}

}  // namespace dsmr
}  // namespace esphome

#endif  // USE_ARDUINO
