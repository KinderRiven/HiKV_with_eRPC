/*
 * @Author: your name
 * @Date: 2021-07-20 14:28:34
 * @LastEditTime: 2021-07-20 19:43:25
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hashtable.hpp
 */

#include "allocator.hpp"
#include "header.hpp"
#include "options.hpp"

namespace hikv {

#define NUM_HASH_SLOT (8)

const int kKeySize = 16;
const int kValueSize = 64;

struct hash_bucket_t {
public:
    uint64_t slot[NUM_HASH_SLOT]; // 8 * 8 = 64B
};

class HashTable {
public:
    HashTable(const Options& options, Allocator* allocator);

    ~HashTable();

    bool Put(const char* key, size_t key_length, uint64_t pos);

    bool Get(const char* key, size_t key_length, char** value, size_t& value_length);

    void Print();

private:
    hash_bucket_t* table_[64];

    uint64_t num_bucket_;

    uint32_t num_partition_;
};

};