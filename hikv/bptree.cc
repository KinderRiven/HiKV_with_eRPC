/*
 * @Author: your name
 * @Date: 2021-07-21 12:56:39
 * @LastEditTime: 2021-07-21 16:15:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/bptree.cc
 */

#include "bptree.hpp"

using namespace hikv;

Bptree::Bptree(Options& options)
{
    btree_ = new ConcurrentBPlusTree<mKey, mValue>();
}

Bptree::~Bptree()
{
}

bool Bptree::Put(mKey& key, mValue& value)
{
    bool _res = btree_->Insert(&key, &value);
    return _res;
}

void Bptree::Print()
{
}