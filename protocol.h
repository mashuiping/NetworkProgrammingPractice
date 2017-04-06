#pragma once

#include <stdint.h>

struct SessionMessage{
  int32_t number;
  int32_t length;
};

struct PayloadMessage{
  int32_t length;
  //usage: PayloadMessage * payload = static_cast<PayloadMessage *>(::malloc(total_len));
  char data[0];
};
