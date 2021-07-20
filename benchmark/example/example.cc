/*
 * @Author: your name
 * @Date: 2021-07-20 15:31:54
 * @LastEditTime: 2021-07-20 19:01:07
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/benchmark/example/example.cc
 */

#include "hikv.hpp"

using namespace hikv;

int main(int argc, char** argv)
{
    Options _options;
    HiKV* _hikv = new HiKV(_options);

    char _key[kKeySize];
    char _value[kValueSize];
    for (int i = 0; i < 1024; i++) {
        *((uint64_t*)_key) = (i + 1);
        *((uint64_t*)_value) = (i + 1);
        _hikv->Put(0, _key, kKeySize, _value, kValueSize);
    }

    char* _get_value;
    size_t _get_value_length;
    for (int i = 0; i < 1024; i++) {
        *((uint64_t*)_key) = (i + 1);
        _hikv->Get(0, _key, kKeySize, &_get_value, _get_value_length);
        printf("[%llu/%llu]\n", *((uint64_t*)_key), *((uint64_t*)_get_value));
    }
    return 0;
}