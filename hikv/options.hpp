/*
 * @Author: your name
 * @Date: 2021-07-20 13:03:28
 * @LastEditTime: 2021-07-20 13:22:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/options.hpp
 */

#ifndef INCLUDE_OPTIONS_HPP_
#define INCLUDE_OPTIONS_HPP_

#include "header.hpp"

namespace hikv {

class Options {
public:
    Options()
        : num_server_threads(4)
        , num_backend_threads(1)
        , index_size(2UL * 1024 * 1024 * 1024)
        , store_size(5UL * 1024 * 1024 * 1024)
        , pmem_file_size(8UL * 1024 * 1024 * 1024)
    {
        strcpy(pmem_file_path, "/home/pmem0/hikv");
    }

public:
    // RPC handle threads
    int num_server_threads;

    // async b+tree insert/update thread
    int num_backend_threads;

    // persistent memory pool size
    size_t pmem_file_size;

    // index size (hashtable)
    size_t index_size;

    // data store size
    size_t store_size;

    // pmem file path
    char pmem_file_path[128];
};

};

#endif