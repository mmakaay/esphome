#pragma once

#include <vector>

namespace esphome {
namespace dsmr {

class DsmrTelegram {
 public:
  explicit DsmrTelegram(const char* type);
  void set_max_length(size_t length) { this->max_length_ = length; }
  virtual void dump_telegram_config();
  virtual void reset();
  virtual bool is_complete() = 0;
  virtual void add(const char byte) = 0;
 protected:
  const char *type_;
  size_t max_length_;
  std::vector<char> data_;
};

}  // namespace dsmr
}  // namespace esphome
