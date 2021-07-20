/*
 * @Author: your name
 * @Date: 2021-07-20 14:28:34
 * @LastEditTime: 2021-07-20 16:08:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hashtable.hpp
 */

#include "allocator.hpp"
#include "header.hpp"
#include "options.hpp"

namespace hikv {

#define NUM_HASH_SLOT (8)

struct hash_bucket_t {
public:
    uint64_t slot[NUM_HASH_SLOT]; // 8 * 8 = 64B
};

class HashTable {
public:
    HashTable(const Options& options, Allocator* allocator);

    ~HashTable();

    bool Put(int thread_id, const char* key, size_t key_length, const char* value, size_t value_length);

    bool Get(int thread_id, const char* key, size_t key_length, char** value, size_t& value_length);

private:
    hash_bucket_t *table_;
};

};