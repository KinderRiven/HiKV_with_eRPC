/*
 * @Author: your name
 * @Date: 2021-07-20 12:56:19
 * @LastEditTime: 2021-07-22 11:06:08
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hikv.cc
 */

#include "hikv.hpp"

using namespace hikv;

static int g_numa[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
// static int g_numa[] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };

static void async_btree_handle(thread_param_t* param)
{
    btree_task_t* _task;
    Bptree* _bptree = param->bptree;
    tbb::concurrent_queue<btree_task_t*>* _queue = param->queue;

    while (true) {
        bool __found = _queue->try_pop(_task);
        if (__found) {
            _bptree->Put(*(_task->mkey), _task->mvalue);
        }
    }
}

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
    // create thread
    for (int i = 0; i < num_backend_thread_; i++) {
        thread_param_t* __param = new thread_param_t();
        __param->num_server_thread = num_sever_thread_;
        __param->thread_id = i;
        __param->bptree = bptree_;
        __param->queue = &task_queue_[i];
        thread_[i] = std::thread(async_btree_handle, __param);
        thread_[i].detach();
    }
}

HiKV::~HiKV()
{
}

bool HiKV::Put(int thread_id, const char* key, size_t key_length, const char* value, size_t value_length)
{
    uint64_t _addr = pstore_[thread_id]->AppendKV(key, key_length, value, value_length);
    bool _res = table_->Put(key, key_length, _addr);
    if (_res) {
        uint32_t _select = CityHash32(key, key_length) % num_backend_thread_;
        task_queue_[_select].push(new btree_task_t(0, key, key_length, _addr));
    }
    return _res;
}

bool HiKV::Get(int thread_id, const char* key, size_t key_length, char** value, size_t& value_length)
{
    bool _res = table_->Get(key, key_length, value, value_length);
    return _res;
}

void HiKV::Print()
{
    // hashtable
    table_->Print();
    // b+tree
    bptree_->Print();
}