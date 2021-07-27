/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-27 10:55:36
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/cs/common.h
 */
#include "rpc.h"
#include <stdio.h>

// --------------- KV packet ------------------------
// | num_kv | key1 | value1 | key 2 | value 2 | ... |
// --------------------------------------------------

static const std::string kServerHostname = "10.0.0.42";
static const std::string kClientHostname = "10.0.0.40";

static constexpr uint16_t kUDPPort = 31850;

static constexpr uint8_t kInsertType = 1;
static constexpr uint8_t kSearchType = 2;

static const uint32_t kNumServerThread = 8;
static const uint32_t kNumClientThread = 4;

static const uint64_t kNumOpt = 10000;

static const uint32_t kNumBatch = 10;
static const uint32_t kHeadSize = 8;
static const uint32_t kKeySize = 16;
static const uint32_t kValueSize = 64;
static const uint32_t kMsgSize = kHeadSize + kKeySize + kValueSize;
