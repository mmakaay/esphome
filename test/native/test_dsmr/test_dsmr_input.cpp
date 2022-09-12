#include <gtest/gtest.h>
#include "components/dsmr/dsmr_input_stub.h"

TEST(dsmr_input_stub, default_is_empty) {
  auto stub = esphome::dsmr::DsmrStubInput();
  ASSERT_FALSE(stub.available()) << "Default stub input is not empty";
}

TEST(dsmr_input_stub, can_provide_one_char_after_0_ms) {
  auto stub = esphome::dsmr::DsmrStubInput();
  stub.add('x');
  ASSERT_TRUE(stub.available()) << "Stub with one char after 0 ms is empty";
  ASSERT_EQ('x', stub.read()) << "Read character is not 'x'";
}

TEST(dsmr_input_stub, can_provide_one_char_after_50_ms) {
  auto stub = esphome::dsmr::DsmrStubInput();
  stub.add('x', 50);
  ASSERT_FALSE(stub.available()) << "Stub with one char after 50 ms is not empty at start";
  ASSERT_FALSE(stub.available()) << "Stub with one char after 50 ms is not empty right after start";
  
  auto start = esphome::dsmr::get_time_in_ms();
  auto passed = 0;
  auto loops = 0;
  bool available = false;
  while (passed <= 100) {
     loops++;
     passed = esphome::dsmr::get_time_in_ms() - start;
    if (stub.available()) {
      available = true;
      break;
    }
  }
  passed = esphome::dsmr::get_time_in_ms() - start;
  ASSERT_FALSE(available) << "Passed: " << passed;
}

// TEST(dsmr_input, flush_clears_all_pending_data) {
//     auto i = esphome::dsmr::DsmrStubInput();
//     EXPECT_TRUE(i.available()) << "No data available in new stub input";
//     i.drain();
//     EXPECT_FALSE(i.available()) << "Data remaining in stub input after drain()";
// }