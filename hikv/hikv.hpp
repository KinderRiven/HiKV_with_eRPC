/*
 * @Author: your name
 * @Date: 2021-07-20 12:56:15
 * @LastEditTime: 2021-07-21 16:26:08
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/hikv.hpp
 */

#ifndef INCLUDE_HIKV_HPP_
#define INCLUDE_HIKV_HPP_

#include "allocator.hpp"
#include "hashtable.hpp"
#include "header.hpp"
#include "options.hpp"
#include "bptree.hpp"
#include "pstore.hpp"

namespace hikv {

class HiKV {
public:
    HiKV(const Options& options);

    ~HiKV();

public:
    bool Put(int thread_id, const char* key, size_t key_length, const char* value, size_t value_length);

    bool Get(int thread_id, const char* key, size_t key_length, char** value, size_t& value_length);

    void Print();

private:
    PStore* pstore_[64];

    HashTable* table_;

    Bptree *bptree_;

    Allocator* allocator_;

    int num_sever_thread_;

    int num_backend_thread_;
};

};

#endif