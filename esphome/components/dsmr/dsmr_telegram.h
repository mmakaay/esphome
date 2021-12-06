#pragma once

#include <vector>

namespace esphome {
namespace dsmr {

class DsmrTelegram {
 public:
  explicit DsmrTelegram(const char* type);
  void set_max_length(size_t length) { this->max_length_ = length; }
  void dump_telegram_config();

  void reset();
  bool is_reading();
  bool is_complete();

  virtual void add(const char byte) = 0;

 protected:
  const char *type_;
  size_t max_length_;
  std::vector<char> data_;
  size_t bytes_read_{0};
  bool is_reading{false};
  bool is_complete{false};
  void set_header_found_();
  void set_footer_found_();
};

enum PlainTextTelegramState {
  TELEGRAM_FIND_HEADER,
  TELEGRAM_FIND_FOOTER,
  TELEGRAM_READ_CRC
};

class DsmrPlainTextTelegram : public DsmrTelegram {
 public:
  explicit DsmrPlainTextTelegram();

  void add(const char byte) override;

 protected:
  PlainTextTelegramState state_{TGRAM_FIND_HEADER};
}

}  // namespace dsmr
}  // namespace esphome
