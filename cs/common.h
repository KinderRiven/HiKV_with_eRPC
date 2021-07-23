/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-23 14:04:22
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/cs/common.h
 */
#include "rpc.h"
#include <stdio.h>

static const std::string kServerHostname = "10.0.0.42";
static const std::string kClientHostname = "10.0.0.40";

static constexpr uint16_t kUDPPort = 31850;

static constexpr uint8_t kInsertType = 1;
static constexpr uint8_t kSearchType = 2;

static const uint32_t kNumServerThread = 8;

static constexpr size_t kMsgSize = 16;
