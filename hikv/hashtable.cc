/*
 * @Author: your name
 * @Date: 2021-07-20 14:28:38
 * @LastEditTime: 2021-07-20 16:10:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hashtable.cc
 */

#include "hashtable.hpp"

using namespace hikv;

HashTable::HashTable(const Options& options, Allocator* allocator)
{
    size_t _index_size = options.index_size / options.num_server_threads;
    table_ = (hash_bucket_t*)allocator->AlignAllocate(256, _index_size);
    printf("HashTable[SIZE:%.2fGB][ADDR:%llu]\n", 1.0 * _index_size / (1024 * 1024 * 1024), (uint64_t)table_);
}

HashTable::~HashTable()
{
}

bool HashTable::Put(int thread_id, const char* key, size_t key_length, const char* value, size_t value_length)
{
}

bool HashTable::Get(int thread_id, const char* key, size_t key_length, char** value, size_t& value_length)
{
}