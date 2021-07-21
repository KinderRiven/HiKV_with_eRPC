/*
 * @Author: your name
 * @Date: 2021-07-20 14:33:08
 * @LastEditTime: 2021-07-21 16:50:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/pstore.hpp
 */

#include "header.hpp"
#include "pmdk/libpmem.h"

#define PMDK_SUPPORT

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
#ifdef PMDK_SUPPORT
    uint64_t AppendValue(const char* buf, size_t buf_size)
    {
        uint64_t _dest = base_ + usage_;
        pmem_memcpy_persist((void*)_dest, (void*)buf, buf_size);
        usage_ += buf_size;
        return _dest;
    }

    uint64_t AppendKV(const char* key, size_t key_length, const char* value, size_t value_length)
    {
        char* _buf = new char[key_length + value_length];
        memcpy((void*)(_buf), key, key_length);
        memcpy((void*)(_buf + key_length), value, value_length);
        uint64_t _dest = base_ + usage_;
        pmem_memcpy_persist((void*)_dest, (void*)_buf, key_length + value_length);
        usage_ += (value_length + key_length);
        return (uint64_t)_buf;
    }
#else
    uint64_t AppendValue(const char* buf, size_t buf_size)
    {
        uint64_t _dest = base_ + usage_;
        memcpy((void*)_dest, (void*)buf, buf_size);
        usage_ += buf_size;
        return _dest;
    }

    uint64_t AppendKV(const char* key, size_t key_length, const char* value, size_t value_length)
    {
        uint64_t _dest1 = base_ + usage_;
        memcpy((void*)_dest1, key, key_length);
        usage_ += key_length;
        uint64_t _dest2 = base_ + usage_;
        memcpy((void*)_dest2, value, value_length);
        usage_ += value_length;
        return _dest1;
    }
#endif

private:
    uint64_t base_;

    uint64_t size_;

    uint64_t usage_;
};

};