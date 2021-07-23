#include <stdio.h>
#include "rpc.h"

static const std::string kServerHostname = "10.0.0.42";
static const std::string kClientHostname = "10.0.0.40";

static constexpr uint16_t kUDPPort = 31850;
static constexpr uint8_t kReqType = 1;
static constexpr size_t kMsgSize = 16;
