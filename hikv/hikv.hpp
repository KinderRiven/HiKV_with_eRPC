/*
 * @Author: your name
 * @Date: 2021-07-20 12:56:15
 * @LastEditTime: 2021-07-21 17:29:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hikv.hpp
 */

#ifndef INCLUDE_HIKV_HPP_
#define INCLUDE_HIKV_HPP_

#include "allocator.hpp"
#include "bptree.hpp"
#include "hashtable.hpp"
#include "header.hpp"
#include "options.hpp"
#include "pstore.hpp"

#include "tbb/concurrent_queue.h"

namespace hikv {

struct btree_task_t {
public:
    int type;
    mKey* mkey;
    mValue mvalue;

public:
    btree_task_t() { }

    btree_task_t(int type, const char* key, size_t key_length, uint64_t pos)
        : type(type)
    {
        mkey = new mKey(key, key_length);
        mvalue = pos;
    }
};

struct thread_param_t {
public:
    int thread_id;
    int num_server_thread;
    Bptree* bptree;
    tbb::concurrent_queue<btree_task_t*>* queue;
};

class HiKV {
public:
    HiKV(const Options& options);

    ~HiKV();

public:
    bool Put(int thread_id, const char* key, size_t key_length, const char* value, size_t value_length);

    bool Get(int thread_id, const char* key, size_t key_length, char** value, size_t& value_length);

    void Print();

private:
    Bptree* bptree_;

    HashTable* table_;

    PStore* pstore_[64];

    Allocator* allocator_;

    std::thread thread_[64];

    tbb::concurrent_queue<btree_task_t*> task_queue_[64];

private:
    int num_sever_thread_;

    int num_backend_thread_;
};

};

#endif