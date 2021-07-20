/*
 * @Author: your name
 * @Date: 2021-07-20 13:52:28
 * @LastEditTime: 2021-07-20 16:05:33
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/allocator.hpp
 */

#ifndef INCLUDE_ALLOCATOR_HPP_
#define INCLUDE_ALLOCATOR_HPP_

#include "header.hpp"
#include "options.hpp"
#include "pmdk/libpmem.h"

namespace hikv {

class Allocator {
public:
    Allocator(const Options& options)
        : usage_(0)
        , size_(options.pmem_file_size)
    {
        int is_pmem;
        size_t mapped_len;
        pmem_map_file(options.pmem_file_path, size_, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
        printf("Allocator::Allocator[PATH:%s][SIZE:%.2fGB/%.2fGB][IS_PMEM:%d]\n",
            options.pmem_file_path, 1.0 * options.pmem_file_size / (1024 * 1024 * 1024),
            1.0 * mapped_len / (1024 * 1024 * 1024), is_pmem);
    }

    ~Allocator()
    {
    }

public:
    void* Allocate(size_t size)
    {
        lock_.lock();
        if (usage_ + size > size_) {
            printf("No enough space!\n");
            exit(0);
        }
        void* _ret = (void*)(base_ + usage_);
        usage_ += size;
        lock_.unlock();
        return _ret;
    }

    void* AlignAllocate(size_t align, size_t size)
    {
        lock_.lock();
        uint64_t _ret = usage_ + base_;
        if (_ret % align != 0) {
            _ret = (_ret + align - 1) / align * align;
            _usage = _ret - base_;
            _ret = usage_ + base_;
        }
        assert(_ret % align == 0);
        usage_ += size;
        lock_.unlock();
        return (void*)(_ret);
    }

private:
    uint64_t base_;

    uint64_t usage_;

    uint64_t size_;

    std::mutex lock_;
};

};

#endif