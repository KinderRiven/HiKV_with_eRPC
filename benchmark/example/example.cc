/*
 * @Author: your name
 * @Date: 2021-07-20 15:31:54
 * @LastEditTime: 2021-07-20 19:14:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/benchmark/example/example.cc
 */

#include "hikv.hpp"
#include <thread>
#include <vector>

using namespace hikv;

void run_put_work(int thread_id, HiKV* hikv, uint64_t low, uint64_t up)
{
    for (int i = low; i <= up; i++) {
        char* __key = (char*)malloc(kKeySize);
        char* __value = (char*)malloc(kValueSize);
        *((uint64_t*)__key) = (i + 1);
        *((uint64_t*)__value) = (i + 1);
        bool _res = hikv->Put(thread_id, __key, kKeySize, __value, kValueSize);
        delete __key;
        delete __value;
    }
}

void run_get_work(int thread_id, HiKV* hikv, uint64_t low, uint64_t up)
{
    for (int i = low; i <= up; i++) {
        char* __key = (char*)malloc(kKeySize);
        char* __value;
        size_t __value_length;
        *((uint64_t*)__key) = (i + 1);
        bool _res = hikv->Get(thread_id, __key, kKeySize, &__value, __value_length);
        delete __key;
        delete __value;
    }
}

int main(int argc, char** argv)
{
    uint64_t _num_kv = 5000000;
    Options _options;
    HiKV* _hikv = new HiKV(_options);
    std::vector<std::thread> _threads(8);

    for (int i = 0; i < _options.num_server_threads; i++) {
        uint64_t __low, __up;
        __low = i * _num_kv + 1;
        __up = (i + 1) * _num_kv + 1;
        _threads[i] = std::thread(run_put_work, i, _hikv, __low, __up);
    }
    for (int i = 0; i < _options.num_server_threads; i++) {
        _threads[i].join();
    }
    return 0;
}