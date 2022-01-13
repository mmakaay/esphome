#include "esphome/core/log.h"
#include "dsmr.h"
#include "dsmr_telegram.h"

namespace esphome {
namespace dsmr {

DsmrTelegram::DsmrTelegram(const char* type) : type_(type) {}

void DsmrTelegram::dump_telegram_config() {
    ESP_LOGCONFIG(TAG, "  Telegram type: %s", type_);
}

void DsmrTelegram::reset() {
  this->data_.clear();
}

}  // namespace dsmr
}  // namespace esphome
