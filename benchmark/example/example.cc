/*
 * @Author: your name
 * @Date: 2021-07-20 15:31:54
 * @LastEditTime: 2021-07-20 15:32:45
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
    return 0;
}