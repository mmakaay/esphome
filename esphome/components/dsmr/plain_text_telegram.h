#pragma once

#include "dsmr_telegram.h"
#include <vector>

namespace esphome {
namespace dsmr {

enum PlainTextTelegramState {
  FindHeader,
  FindFooter,
  ReadCRC,
  Complete
};

class PlainTextTelegram : public DsmrTelegram {
 public:
  explicit PlainTextTelegram();
  void reset() override;
  void add(const char byte) override;
  bool is_complete() override;
 protected:
  bool is_reading_;
  std::vector<char> crc_;
  PlainTextTelegramState state_;
};

}  // namespace dsmr
}  // namespace esphome
