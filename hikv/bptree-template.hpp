#ifndef INCLUDE_BPTREE_TEMP_H_
#define INCLUDE_BPTREE_TEMP_H_

#include "header.hpp"
#include "tbb/spin_mutex.h"

/* Concurrency control based on hardware transaction memory, 
   more detail please refer to the paper for the specific algorithm 
   <FPTree: A Hybrid SCM-DRAM Persistent and Concurrent B-Tree for Storage Class Memory> */

namespace hikv {

template <typename Key, typename Value>
class ConcurrentBPlusTree {
public:
    struct KVEntry {
    public:
        Key key;
        Value value;
    };

    struct Node { //16B
    public:
        int is_leaf; // 4B
        int nkeys; // 4B
        Node* parent; // 8B

    public:
        inline bool is_leaf_node()
        {
            return is_leaf;
        }
    };

    static const int do_line_search = 0;
    static const int node_size = 1024;
    static const int inner_node_slots = (1024 - sizeof(Node) - 8) / (sizeof(Key) + sizeof(Node*));
    static const int leaf_node_slots = (1024 - sizeof(Node) - 9) / (sizeof(KVEntry));

    struct InnerNode : public Node {
    public:
        Key keys[inner_node_slots];
        Node* left_most_pointer;
        Node* child_pointers[inner_node_slots];

    private:
        inline Node* line_search_down(Key& key)
        {
            int i;
            if (key < keys[0]) {
                assert(left_most_pointer != nullptr);
                return left_most_pointer;
            }
            for (i = 1; i < this->nkeys; i++) {
                if (key < keys[i]) {
                    break;
                }
            }
            return child_pointers[i - 1];
        }

        inline Node* binary_search_down(Key& key)
        {
            /* binary search 
               upper_bound() */
            int l = 0, h = this->nkeys;
            while (l < h) {
                int mid = l + (h - l) / 2;
                if (key >= keys[mid]) {
                    l = mid + 1;
                } else {
                    h = mid;
                }
            }
            return (l == 0) ? left_most_pointer : child_pointers[l - 1];
        }

        inline bool line_insert(Key& key, Node* new_child)
        {
            int i = 0;
            /* 1.line search */
            for (i = 0; i < this->nkeys; i++) {
                if (key <= keys[i]) {
                    break;
                }
            }
            /* 2.move entries */
            for (int j = this->nkeys; j > i; j--) {
                keys[j] = keys[j - 1];
                child_pointers[j] = child_pointers[j - 1];
            }
            keys[i] = key;
            child_pointers[i] = new_child;
            this->nkeys++;
            return true;
        }

        inline bool binary_insert(Key& key, Node* new_child)
        {
            /* binary search 
               upper_bound() */
            int l = 0, h = this->nkeys;
            while (l < h) {
                int mid = l + (h - l) / 2;
                if (key >= keys[mid]) {
                    l = mid + 1;
                } else {
                    h = mid;
                }
            }
            /* 2.move entries */
            for (int j = this->nkeys; j > l; j--) {
                keys[j] = keys[j - 1];
                child_pointers[j] = child_pointers[j - 1];
            }
            keys[l] = key;
            child_pointers[l] = new_child;
            this->nkeys++;
            return true;
        }

    public:
        inline void initialize()
        {
            this->is_leaf = false;
            this->nkeys = 0;
            this->parent = nullptr;
            left_most_pointer = nullptr;
        }

        inline bool is_full()
        {
            return (this->nkeys == inner_node_slots) ? true : false;
        }

        inline void print()
        {
            printf("<%d>", this->nkeys);
            for (int i = 0; i < this->nkeys; i++) {
                printf("[%llu]", keys[i].Number());
            }
            printf("\n");
        }

        inline Node* search_down(Key& key)
        {
            if (do_line_search) {
                return line_search_down(key);
            } else {
                return binary_search_down(key);
            }
        }

        inline bool insert(Key& key, Node* new_child)
        {
            if (do_line_search) {
                return line_insert(key, new_child);
            } else {
                return binary_insert(key, new_child);
            }
        }

    } __attribute__((aligned(64)));

    struct LeafNode : public Node {
    public:
        KVEntry kv_entries[leaf_node_slots];
        LeafNode* next; // 8
        unsigned char mlock; // 1

    private:
        inline bool line_search(Key& key, Value& value)
        {
            int i;
            for (i = 0; i < this->nkeys; i++) {
                if (kv_entries[i].key >= key) {
                    break;
                }
            }
            if (kv_entries[i].key == key) {
                value = kv_entries[i].value;
                return true;
            } else {
                return false;
            }
        }

        inline bool binary_search(Key& key, Value& value)
        {
            /* binary search 
               lower_bound() */
            int l = 0, h = this->nkeys; // Not n - 1
            while (l < h) {
                int mid = l + (h - l) / 2;
                if (key <= kv_entries[mid].key) {
                    h = mid;
                } else {
                    l = mid + 1;
                }
            }
            if ((l < this->nkeys) && (key == kv_entries[l].key)) {
                value = kv_entries[l].value;
                return true;
            } else {
                return false;
            }
        }

        inline bool line_insert(Key& key, Value& value)
        {
            int i = 0;
            /* 1.line search */
            for (i = 0; i < this->nkeys; i++) {
                if (key <= kv_entries[i].key) {
                    break;
                }
            }
            /* 2.1 only-update */
            if (kv_entries[i].key == key) {
                kv_entries[i].value = value;
            } else {
                /* 2.2 insert */
                for (int j = this->nkeys; j > i; j--) {
                    kv_entries[j] = kv_entries[j - 1];
                }
                kv_entries[i].key = key;
                kv_entries[i].value = value;
                this->nkeys++;
            }
            return true;
        }

        inline bool binary_insert(Key& key, Value& value)
        {
            /* binary search 
               lower_bound() */
            int l = 0, h = this->nkeys; // Not n - 1
            while (l < h) {
                int mid = l + (h - l) / 2;
                if (key <= kv_entries[mid].key) {
                    h = mid;
                } else {
                    l = mid + 1;
                }
            }
            if ((l < this->nkeys) && (key == kv_entries[l].key)) {
                kv_entries[l].value = value;
            } else {
                for (int j = this->nkeys; j > l; j--) {
                    kv_entries[j] = kv_entries[j - 1];
                }
                kv_entries[l].key = key;
                kv_entries[l].value = value;
                this->nkeys++;
            }
            return true;
        }

        inline bool line_remove(Key& key)
        {
            int i;
            for (i = 0; i < this->nkeys; i++) {
                if (kv_entries[i].key >= key) {
                    break;
                }
            }
            if (kv_entries[i].key == key) {
                kv_entries[i].value = 0;
                return true;
            } else {
                return false;
            }
        }

        inline bool binary_remove(Key& key)
        {
            /* binary search 
               lower_bound() */
            int l = 0, h = this->nkeys; // Not n - 1
            while (l < h) {
                int mid = l + (h - l) / 2;
                if (key <= kv_entries[mid].key) {
                    h = mid;
                } else {
                    l = mid + 1;
                }
            }
            if ((l < this->nkeys) && (key == kv_entries[l].key)) {
                kv_entries[l].value = 0;
                return true;
            } else {
                return false;
            }
        }

    public:
        inline void initialize()
        {
            memset(kv_entries, 0, sizeof(kv_entries));
            this->is_leaf = true;
            this->nkeys = 0;
            this->parent = nullptr;
            next = nullptr;
            mlock = 0;
        }

        inline bool is_full()
        {
            return (this->nkeys == leaf_node_slots) ? true : false;
        }

        inline void print()
        {
            printf("{%d}", this->nkeys);
            for (int i = 0; i < this->nkeys; i++) {
                printf("[%llu]", kv_entries[i].key.Number());
            }
            printf("\n");
        }

        inline bool search(Key& key, Value& value)
        {
            if (do_line_search) {
                return line_search(key, value);
            } else {
                return binary_search(key, value);
            }
        }

        inline bool remove(Key& key)
        {
            if (do_line_search) {
                return line_remove(key);
            } else {
                return binary_remove(key);
            }
        }

        inline bool insert(Key& key, Value& value)
        {
            if (do_line_search) {
                return line_insert(key, value);
            } else {
                return binary_insert(key, value);
            }
        }

        inline int scan_all(std::vector<std::pair<Key, Value>>& result)
        {
            for (int i = 0; i < this->nkeys; i++) {
                result.push_back(std::pair<Key, Value>(kv_entries[i].key, kv_entries[i].value));
            }
            return this->nkeys;
        }

    public:
        inline void lock()
        {
            mlock = 1;
        }

        inline void unlock()
        {
            mlock = 0;
        }

        inline bool is_lock()
        {
            return (mlock == 1) ? true : false;
        }

    } __attribute__((aligned(64)));

public:
    ConcurrentBPlusTree()
    {
        root_ = (Node*)alloc_leaf_node();
        header_ = (LeafNode*)root_;
    }

    ~ConcurrentBPlusTree()
    {
    }

public:
    bool Insert(Key* key, Value* value)
    {
        bptree_insert(*key, *value);
        return true;
    }

    bool Search(Key* key, Value* value)
    {
        bool result = bptree_search(*key, *value);
        return result;
    }

    bool Remove(Key* key, Value* value)
    {
        bool result = bptree_remove(*key);
        return result;
    }

    bool Scan(Key* key, int range, std::vector<std::pair<Key, Value>>& vec_pair)
    {
        int n = bptree_scan(*key, range, vec_pair);
        return true;
    }

    void Print()
    {
        max_depth = 0;
        num_kv = 0;
        num_next_node = 0;
        num_leaf_node = 0;
        num_inner_node = 0;

        /* do print */
        if (do_line_search) {
            printf(">>[B+Tree_Line_Search]\n");
        } else {
            printf(">>[B+Tree_Binary_Searhc]\n");
        }

        /* do next search */
        bptree_line_search();
        printf("  [num_next_node:%llu]\n", num_next_node);

        /* do dfs search*/
        bptree_dfs_search(0, root_);
        printf("  [depth:%llu][kv_count:%llu]\n", max_depth, num_kv);
        printf("  [inner_node:%llu][size:%zuB][slot:%d]\n", num_inner_node, sizeof(InnerNode), inner_node_slots);
        printf("  [leaf_node:%llu][size:%zuB][slot:%d]\n", num_leaf_node, sizeof(LeafNode), leaf_node_slots);
    }

public:
    class Iterator {
    public:
        friend class ConcurrentBPlusTree<Key, Value>;

    public:
        uint32_t pos_;
        LeafNode* cur_;

    public:
        explicit Iterator(LeafNode* leaf, uint32_t pos)
            : cur_(leaf)
            , pos_(pos)
        {
        }

        void Next()
        {
            while (true) {
                pos_++;
                if (pos_ >= cur_->nkeys) {
                    cur_ = cur_->next;
                    pos_ = 0;
                }
                if (cur_ == nullptr) {
                    break;
                }
                if (cur_->kv_entries[pos_].value != 0) {
                    break;
                }
            }
        }

        void Prev()
        {
            // TODO
        }

        bool Vaild()
        {
            return (cur_ != nullptr) ? true : false;
        }

        Key GetKey()
        {
            if (cur_ != nullptr) {
                return cur_->kv_entries[pos_].key;
            } else {
                printf("concurrent_btree::iterator->bad key!\n");
            }
        }

        Value GetValue()
        {
            if (cur_ != nullptr) {
                return cur_->kv_entries[pos_].value;
            } else {
                printf("concurrent_btree::iterator->basd value!\n");
            }
        }
    };

public:
    Iterator* Begin()
    {
        uint32_t _pos;
        LeafNode* _leaf = header_;
        while (true) {
            if (_leaf == nullptr) {
                break;
            }
            if (_leaf->kv_entries[_pos].value != 0) {
                break;
            }
            _pos++;
            if (_pos >= _leaf->nkeys) {
                _leaf = _leaf->next;
                _pos = 0;
            }
        }
        Iterator* _iter = new Iterator(_leaf, _pos);
        return _iter;
    }

    Iterator* End()
    {
        // TODO
    }

private:
    InnerNode* alloc_inner_node()
    {
        InnerNode* inner_node = (InnerNode*)malloc(sizeof(InnerNode));
        inner_node->initialize();
        return inner_node;
    }

    LeafNode* alloc_leaf_node()
    {
        LeafNode* leaf_node = (LeafNode*)malloc(sizeof(LeafNode));
        leaf_node->initialize();
        return leaf_node;
    }

private:
    LeafNode* search_leaf_node(Node* cur, Key& key)
    {
        if (cur->is_leaf_node()) {
            return (LeafNode*)cur;
        } else {
            cur = ((InnerNode*)cur)->search_down(key);
            return search_leaf_node(cur, key);
        }
    }

    bool insert_into_leaf_node(LeafNode* leaf_node, Key& key, Value& value)
    {
        leaf_node->insert(key, value);
        return true;
    }

    bool insert_into_inner_node(InnerNode* inner_node, Key& key, Node* child)
    {
        inner_node->insert(key, child);
        return true;
    }

    bool insert_into_parent(InnerNode* parent_node, Key& min_key, Node* new_node)
    {
        if (parent_node == nullptr) {
            /* 1. alloca new root */
            InnerNode* new_root = alloc_inner_node();

            /* 2. insert split_key */
            new_root->keys[0] = min_key;

            /* 3. modify child pointer*/
            new_root->left_most_pointer = root_;
            new_root->child_pointers[0] = new_node;

            /* 4. establish parent-child link */
            root_->parent = (Node*)new_root;
            new_node->parent = (Node*)new_root;

            /* 5. okkkkkk */
            new_root->nkeys = 1;
            root_ = new_root;
        } else if (parent_node->is_full()) {
            /* 1. allocate new inner node */
            InnerNode* new_inner_node = alloc_inner_node();

            /* 2. move */
            int nkeys = parent_node->nkeys;
            int pos = (nkeys >> 1);
            for (int i = pos, j = 0; i < parent_node->nkeys; i++, j++) {
                new_inner_node->keys[j] = parent_node->keys[i];
                new_inner_node->child_pointers[j] = parent_node->child_pointers[i];
                /* warnning : Pay attention to modify the pointer,
                   Something error happened in here T^T */
                parent_node->child_pointers[i]->parent = new_inner_node;
                new_inner_node->nkeys++;
                nkeys--;
            }
            parent_node->nkeys = nkeys;

            /* 3. insert new entry */
            if (min_key < new_inner_node->keys[0]) {
                insert_into_inner_node(parent_node, min_key, new_node);
                new_node->parent = parent_node;
            } else {
                insert_into_inner_node(new_inner_node, min_key, new_node);
                new_node->parent = new_inner_node;
            }

            /* 4. update parent */
            insert_into_parent((InnerNode*)parent_node->parent, new_inner_node->keys[0], (Node*)new_inner_node);
        } else {
            parent_node->insert(min_key, new_node);
            new_node->parent = (Node*)parent_node;
        }
        return true;
    }

    bool bptree_insert(Key& key, Value& value)
    {
        LeafNode* leaf_node = nullptr;

        while (true) {
            tbb::speculative_spin_mutex::scoped_lock speculative_lock(mutex_);
            leaf_node = search_leaf_node(root_, key);
            if (leaf_node->is_lock()) {
                _xabort(0xff);
                continue;
            }
            leaf_node->lock();
            speculative_lock.release();
            break;
        }

        if (leaf_node->is_full()) { // need to split
            /* 1.allocate new leaf node */
            LeafNode* new_leaf_node = alloc_leaf_node();

            /* 2.move half items to new leaf node */
            int nkeys = leaf_node->nkeys;
            int pos = (leaf_node->nkeys >> 1);
            for (int i = pos, j = 0; i < leaf_node->nkeys; i++, j++) {
                new_leaf_node->kv_entries[j] = leaf_node->kv_entries[i];
                new_leaf_node->nkeys++;
                nkeys--;
            }
            leaf_node->nkeys = nkeys;

            /* 3.insert into new kv */
            if (key < new_leaf_node->kv_entries[0].key) {
                insert_into_leaf_node(leaf_node, key, value);
            } else {
                insert_into_leaf_node(new_leaf_node, key, value);
            }

            /* 4.change pointer */
            new_leaf_node->next = leaf_node->next;
            leaf_node->next = new_leaf_node;

            /* 5.update inner node */
            tbb::speculative_spin_mutex::scoped_lock speculative_lock(mutex_);
            insert_into_parent((InnerNode*)leaf_node->parent, new_leaf_node->kv_entries[0].key, (Node*)new_leaf_node);
            speculative_lock.release();
        } else { // directly insert
            insert_into_leaf_node(leaf_node, key, value);
        }
        leaf_node->unlock();
        return true;
    }

    bool bptree_search(Key& key, Value& value)
    {
        bool result = false;
        while (true) {
            tbb::speculative_spin_mutex::scoped_lock speculative_lock(mutex_);
            LeafNode* leaf_node = search_leaf_node(root_, key);
            if (leaf_node->is_lock()) {
                _xabort(0xff);
                continue;
            }
            result = leaf_node->search(key, value);
            speculative_lock.release();
            break;
        }
        return result;
    }

    bool bptree_remove(Key& key)
    {
        bool result = false;
        while (true) {
            tbb::speculative_spin_mutex::scoped_lock speculative_lock(mutex_);
            LeafNode* leaf_node = search_leaf_node(root_, key);
            if (leaf_node->is_lock()) {
                _xabort(0xff);
                continue;
            }
            result = leaf_node->remove(key);
            speculative_lock.release();
            break;
        }
        return result;
    }

    int bptree_scan(Key& key, int count, std::vector<std::pair<Key, Value>>& vec_result)
    {
        int n = 0;
        LeafNode* leaf_node = search_leaf_node(root_, key);
        while (leaf_node != nullptr) {
            n += leaf_node->scan_all(vec_result);
            leaf_node = leaf_node->next;
            if (n >= count) {
                break;
            }
        }
        return n;
    }

    void bptree_print(int depth, Node* cur)
    {
        for (int i = 0; i < depth; i++) {
            printf("+-");
        }
        if (cur->is_leaf_node()) {
            ((LeafNode*)cur)->print();
        } else {
            InnerNode* c = (InnerNode*)cur;
            c->print();
            if (c->left_most_pointer != nullptr) {
                bptree_print(depth + 1, c->left_most_pointer);
            }
            for (int i = 0; i < c->nkeys; i++) {
                bptree_print(depth + 1, c->child_pointers[i]);
            }
        }
    }

    void bptree_dfs_search(int depth, Node* cur)
    {
        max_depth = (max_depth < depth) ? depth : max_depth;
        if (cur->is_leaf_node()) {
            LeafNode* lnode = (LeafNode*)cur;
            for (int i = 0; i < lnode->nkeys; i++) {
                if (lnode->kv_entries[i].value != 0) {
                    num_kv++;
                }
            }
            // num_kv += cur->nkeys;
            num_leaf_node++;
        } else {
            num_inner_node++;
            InnerNode* c = (InnerNode*)cur;
            if (c->left_most_pointer != nullptr) {
                bptree_dfs_search(depth + 1, c->left_most_pointer);
            }
            for (int i = 0; i < c->nkeys; i++) {
                bptree_dfs_search(depth + 1, c->child_pointers[i]);
            }
        }
    }

    void bptree_line_search()
    {
        uint32_t _kv_cnt = 0;
        LeafNode* cur = header_;
        while (cur != nullptr) {
            for (int i = 0; i < cur->nkeys; i++) {
                if (cur->kv_entries[i].value != 0) {
                    // printf("%llu\n", cur->kv_entries[i].key.Number());
                    _kv_cnt++;
                }
            }
            num_next_node++;
            cur = cur->next;
        }
        // printf("bptree_line_search [kv_cnt:%llu]\n", _kv_cnt);
    }

private:
    tbb::speculative_spin_mutex mutex_;

    Node* root_;

    LeafNode* header_;

private: // DEBUG_PRINT_MSG
    uint64_t max_depth;

    uint64_t num_kv;

    uint64_t num_next_node;

    uint64_t num_leaf_node;

    uint64_t num_inner_node;
};
};

#endif