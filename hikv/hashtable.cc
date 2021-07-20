/*
 * @Author: your name
 * @Date: 2021-07-20 14:28:38
 * @LastEditTime: 2021-07-20 19:20:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hashtable.cc
 */

#include "hashtable.hpp"
#include "city.h"

using namespace hikv;

HashTable::HashTable(const Options& options, Allocator* allocator)
{
    num_partition_ = options.num_server_threads;
    size_t _index_size = options.index_size / num_partition_;
    num_bucket_ = _index_size / sizeof(hash_bucket_t);

    for (int i = 0; i < num_partition_; i++) {
        table_[i] = (hash_bucket_t*)allocator->AlignAllocate(256, _index_size);
        memset(table_[i], 0, _index_size);
        printf("HashTable[%02d][SIZE:%.2fGB][ADDR:%llu][NUM_BUCKET:%llu]\n",
            i, 1.0 * _index_size / (1024 * 1024 * 1024), (uint64_t)table_[i], num_bucket_);
    }
}

HashTable::~HashTable()
{
}

bool HashTable::Put(const char* key, size_t key_length, uint64_t pos)
{
    bool _flag = false;
    uint64_t _hash64 = CityHash64(key, key_length);
    uint64_t _bucket_id = _hash64 % num_bucket_;
    uint32_t _partition_id = ((_bucket_id >> 56) & 0xff);
    uint8_t _signature = ((_bucket_id >> 48) & 0xff);
    hash_bucket_t* _bucket = table_[_partition_id] + _bucket_id;

    for (int i = 0; i < NUM_HASH_SLOT; i++) {
        if (_bucket->slot[i]) {
            uint64_t __slot = _bucket->slot[i];
            uint8_t __signature = ((__slot >> 48) & 0xff);
            if (__signature == _signature) {
                uint64_t __addr = (_bucket->slot[i] & 0xffffffffffff);
                if (memcmp((void*)key, (void*)__addr, kKeySize) == 0) {
                    __slot = (((uint64_t)_signature << 48) | pos);
                    _bucket->slot[i] = __slot;
                    _flag = true;
                    break;
                }
            }
        } else {
            uint64_t __slot = (((uint64_t)_signature << 48) | pos);
            bool __flag = __sync_bool_compare_and_swap(&_bucket->slot[i], 0, __slot);
            if (__flag) {
                _flag = true;
                break;
            }
        }
    }
    return _flag;
}

bool HashTable::Get(const char* key, size_t key_length, char** value, size_t& value_length)
{
    bool _flag = false;
    uint64_t _hash64 = CityHash64(key, key_length);
    uint64_t _bucket_id = _hash64 % num_bucket_;
    uint32_t _partition_id = ((_bucket_id >> 56) & 0xff);
    uint8_t _signature = ((_bucket_id >> 48) & 0xff);
    hash_bucket_t* _bucket = table_[_partition_id] + _bucket_id;

    for (int i = 0; i < NUM_HASH_SLOT; i++) {
        if (_bucket->slot[i]) {
            uint64_t __slot = _bucket->slot[i];
            uint8_t __signature = ((__slot >> 48) & 0xff);
            if (__signature == _signature) {
                uint64_t __addr = (_bucket->slot[i] & 0xffffffffffff);
                if (memcmp((void*)key, (void*)__addr, kKeySize) == 0) {
                    *value = (char*)malloc(kValueSize);
                    value_length = kValueSize;
                    memcpy((void*)(*value), (void*)__addr, value_length);
                    _flag = true;
                    break;
                }
            }
        }
    }
    return _flag;
}