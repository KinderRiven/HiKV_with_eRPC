/*
 * @Author: your name
 * @Date: 2021-07-20 14:33:08
 * @LastEditTime: 2021-07-20 14:47:03
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/pstore.hpp
 */

#include "header.hpp"
#include "pmdk/libpmem.h"

namespace hikv {

class PStore {
public:
    PStore(uint64_t base, size_t size)
        : base_(base)
        , size_(size)
        , usage_(0)
    {
        printf("PStore[base:%llu][%.2fGB]\n", base_, 1.0 * size_ / (1024 * 1024 * 1024));
    }

    ~PStore()
    {
    }

public:
    uint64_t Append(const char* buf, size_t buf_size)
    {
        uint64_t _dest = base_ + usage_;
        memcpy((void*)_dest, (void*)buf, buf_size);
        usage_ += buf_size;
        return _dest;
    }

private:
    uint64_t base_;

    uint64_t size_;

    uint64_t usage_;
};

};