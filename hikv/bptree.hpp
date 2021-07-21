/*
 * @Author: your name
 * @Date: 2021-07-21 11:03:50
 * @LastEditTime: 2021-07-21 16:29:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/hikv/bptree.hpp
 */

#ifndef INCLUDE_BPTREE_H_
#define INCLUDE_BPTREE_H_

#include "bptree-template.hpp"
#include "header.hpp"
#include "options.hpp"

namespace hikv {

#define KEY_BUF_SIZE (16)

class mKey {
public:
    mKey()
    {
    }

    mKey(char* str)
    {
        memcpy(buf_, str, strlen(str));
    }

    mKey(char* str, size_t len)
    {
        memcpy(buf_, str, len);
    }

    mKey(std::string string)
    {
        memcpy(buf_, string.data(), string.size());
    }

    mKey(uint64_t num)
    {
      *((uint64_t *)buf_) = num;
    }

    mKey(const mKey& fstring)
    {
        memcpy(buf_, fstring.buf_, KEY_BUF_SIZE);
    }

    ~mKey()
    {
    }

public:
    inline char* Data()
    {
        return buf_;
    }

    inline size_t Length()
    {
        return KEY_BUF_SIZE;
    }

    inline const char* Buf()
    {
        return buf_;
    }

    inline const size_t Size()
    {
        return KEY_BUF_SIZE;
    }

public:
    int Compare(const mKey& fstring)
    {
        return memcmp(buf_, fstring.buf_, KEY_BUF_SIZE);
    }

    mKey& operator=(const mKey& fstring)
    {
        memcpy(buf_, fstring.buf_, KEY_BUF_SIZE);
        return *this;
    }

    bool operator==(const mKey& fstring) const
    {
        return (memcmp(buf_, fstring.buf_, KEY_BUF_SIZE) == 0) ? true : false;
    }

    bool operator<(const mKey& fstring) const
    {
        return (memcmp(buf_, fstring.buf_, KEY_BUF_SIZE) < 0) ? true : false;
    }

    bool operator<=(const mKey& fstring) const
    {
        return (memcmp(buf_, fstring.buf_, KEY_BUF_SIZE) <= 0) ? true : false;
    }

    bool operator>(const mKey& fstring) const
    {
        return (memcmp(buf_, fstring.buf_, KEY_BUF_SIZE) > 0) ? true : false;
    }

    bool operator>=(const mKey& fstring) const
    {
        return (memcmp(buf_, fstring.buf_, KEY_BUF_SIZE) >= 0) ? true : false;
    }

private:
    char buf_[KEY_BUF_SIZE] = { 0 };
};

typedef uint64_t mValue;

class Bptree {
public:
    Bptree(const Options& options);

    ~Bptree();

    bool Put(mKey &key, mValue &value);

    void Print();

private:
    ConcurrentBPlusTree<mKey, mValue> *btree_;
};

}

#endif