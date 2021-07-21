/*
 * @Author: your name
 * @Date: 2021-07-20 15:31:54
 * @LastEditTime: 2021-07-21 18:49:27
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/benchmark/example/example.cc
 */

#include "hikv.hpp"
#include "timer.h"
#include <thread>
#include <vector>

using namespace hikv;

static int g_numa[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
// static int g_numa[] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };

double g_iops[64] = { 0 };

void run_put_work(int thread_id, HiKV* hikv, uint64_t low, uint64_t up)
{
#if 1
    cpu_set_t _mask;
    CPU_ZERO(&_mask);
    CPU_SET(g_numa[thread_id], &_mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(_mask), &_mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }
#endif

    Timer _timer;
    _timer.Start();
    for (int i = low; i <= up; i++) {
        char* __key = (char*)new char[kKeySize];
        char* __value = (char*)new char[kValueSize];
        *((uint64_t*)__key) = (i + 1);
        *((uint64_t*)__value) = (i + 1);
        bool _res = hikv->Put(thread_id, __key, kKeySize, __value, kValueSize);
        delete __key;
        delete __value;
    }
    _timer.Stop();
    double _iops = 1.0 * (up - low + 1) / _timer.GetSeconds();
    g_iops[thread_id] = _iops;
    printf("[%d][IOPS:%.2f]\n", thread_id, _iops);
}

void run_get_work(int thread_id, HiKV* hikv, uint64_t low, uint64_t up)
{
#if 1
    cpu_set_t _mask;
    CPU_ZERO(&_mask);
    CPU_SET(g_numa[thread_id], &_mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(_mask), &_mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }
#endif

    uint64_t _found = 0;
    Timer _timer;
    _timer.Start();
    for (int i = low; i <= up; i++) {
        char* __key = (char*)new char[kKeySize];
        char* __value = nullptr;
        size_t __value_length;
        *((uint64_t*)__key) = (i + 1);
        bool _res = hikv->Get(thread_id, __key, kKeySize, &__value, __value_length);
        if (_res) {
            if (*(uint64_t*)__key == *(uint64_t*)__value) {
                _found++;
            }
        }
        delete __key;
        delete __value;
    }
    _timer.Stop();
    double _iops = 1.0 * (up - low + 1) / _timer.GetSeconds();
    g_iops[thread_id] = _iops;
    printf("[%d][IOPS:%.2f][FOUND:%llu/%llu]\n", thread_id, _iops, _found, up - low + 1);
}

int main(int argc, char** argv)
{
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    uint64_t _num_kv = 50000000;
    Options _options;
    _options.pmem_file_size = 100UL * (1024 * 1024 * 1024);
    _options.index_size = 32UL * (1024 * 1024 * 1024);
    _options.store_size = 64UL * (1024 * 1024 * 1024);
    _options.num_server_threads = 8;
    _num_kv /= _options.num_server_threads;
    strcpy(_options.pmem_file_path, "/home/pmem3/hikv");
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    double _sum_iops;
    HiKV* _hikv = new HiKV(_options);
    std::vector<std::thread> _threads(_options.num_server_threads);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < _options.num_server_threads; i++) {
        uint64_t __low, __up;
        __low = i * _num_kv + 1;
        __up = (i + 1) * _num_kv;
        _threads[i] = std::thread(run_put_work, i, _hikv, __low, __up);
    }
    for (int i = 0; i < _options.num_server_threads; i++) {
        _threads[i].join();
    }
    _sum_iops = 0;
    for (int i = 0; i < _options.num_server_threads; i++) {
        _sum_iops += g_iops[i];
    }
    printf("[SUM][IOPS:%.2f]\n", _sum_iops);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < _options.num_server_threads; i++) {
        uint64_t __low, __up;
        __low = i * _num_kv + 1;
        __up = (i + 1) * _num_kv + 1;
        _threads[i] = std::thread(run_get_work, i, _hikv, __low, __up);
    }
    for (int i = 0; i < _options.num_server_threads; i++) {
        _threads[i].join();
    }
    _sum_iops = 0;
    for (int i = 0; i < _options.num_server_threads; i++) {
        _sum_iops += g_iops[i];
    }
    printf("[SUM][IOPS:%.2f]\n", _sum_iops);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    _hikv->Print();
    return 0;
}