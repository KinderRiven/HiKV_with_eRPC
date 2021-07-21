/*
 * @Author: your name
 * @Date: 2021-07-20 12:56:19
 * @LastEditTime: 2021-07-21 16:26:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hikv.cc
 */

#include "hikv.hpp"

using namespace hikv;

HiKV::HiKV(const Options& options)
    : num_sever_thread_(options.num_server_threads)
    , num_backend_thread_(options.num_backend_threads)
{
    // NVM Allocator
    allocator_ = new Allocator(options);

    // KV Store
    for (int i = 0; i < num_sever_thread_; i++) {
        void* __base = allocator_->Allocate(options.store_size / num_sever_thread_);
        pstore_[i] = new PStore((uint64_t)__base, options.store_size / num_sever_thread_);
    }

    // hashtable
    table_ = new HashTable(options, allocator_);

    // b+tree
    bptree_ = new Bptree(options);
}

HiKV::~HiKV()
{
}

bool HiKV::Put(int thread_id, const char* key, size_t key_length, const char* value, size_t value_length)
{
    uint64_t _addr = pstore_[thread_id]->AppendKV(key, key_length, value, value_length);
    bool _res = table_->Put(key, key_length, _addr);
    return _res;
}

bool HiKV::Get(int thread_id, const char* key, size_t key_length, char** value, size_t& value_length)
{
    bool _res = table_->Get(key, key_length, value, value_length);
    return _res;
}

void HiKV::Print()
{
    table_->Print();
    bptree_->Print();
}