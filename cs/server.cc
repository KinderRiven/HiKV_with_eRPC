/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-27 12:50:04
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /code/eRPC/hello_world/server.cc
 */

#include "common.h"
#include "hikv.hpp"
#include <thread>

static int g_numa[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
// static int g_numa[] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };

struct ServerContext {
public:
    int thread_id;
    erpc::Nexus* nexus;
    hikv::HiKV* hikv;
    erpc::Rpc<erpc::CTransport>* rpc = nullptr;
};

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

void req_insert_handle(erpc::ReqHandle* req_handle, void* context)
{
    ServerContext* _context = (ServerContext*)context;
    auto _req = req_handle->get_req_msgbuf();
    char* _buf = (char*)_req->buf;
    uint64_t _num_batch = *(uint64_t*)_buf;
    _buf += kHeadSize;
    assert(_num_batch == kNumBatch);

    for (int i = 0; i < _num_batch; i++) {
        char* _skey = (char*)_buf;
        _buf += kKeySize;
        char* _svalue = (char*)_buf;
        _buf += kValueSize;
        hikv::HiKV* _hikv = _context->hikv;
        bool _res = _hikv->Put(_context->thread_id, _skey, kKeySize, _svalue, kValueSize);
    }

    // Respnse
    auto& resp = req_handle->pre_resp_msgbuf;
    _context->rpc->resize_msg_buffer(&resp, 8);
    *(uint64_t*)resp.buf = _num_batch;
    _context->rpc->enqueue_response(req_handle, &resp);
}

void req_search_handle(erpc::ReqHandle* req_handle, void* context)
{
    uint64_t _num_kv = 0;
    ServerContext* _context = (ServerContext*)context;
    hikv::HiKV* _hikv = _context->hikv;

    auto _req = req_handle->get_req_msgbuf();
    char* _buf = (char*)_req->buf;
    uint64_t _num_batch = *(uint64_t*)_buf;
    _buf += kHeadSize;
    assert(_num_batch == kNumBatch);

    auto& resp = req_handle->pre_resp_msgbuf;
    _context->rpc->resize_msg_buffer(&resp, kMsgSize * _num_batch);
    char* _resp_buf = (char*)resp.buf;
    char* _resp_header = _resp_buf;
    _resp_buf += kHeadSize;

    for (int i = 0; i < _num_batch; i++) {
        char* _skey = (char*)_buf;
        _buf += kKeySize;
        char* _hikv_value = nullptr;
        size_t _hikv_value_length;
        bool _res = _hikv->Get(_context->thread_id, _skey, kKeySize, &_hikv_value, _hikv_value_length);
        if (_res) {
            memcpy(_resp_buf, _skey, kKeySize);
            _resp_buf += kKeySize;
            memcpy(_resp_buf, _hikv_value, _hikv_value_length);
            _resp_buf += kValueSize;
            delete _hikv_value;
            _num_kv++;
        }
    }
    *(uint64_t*)_resp_header = _num_kv;
    _context->rpc->enqueue_response(req_handle, &resp);
}

static void run_server_thread(ServerContext* context)
{
    int _thread_id = context->thread_id;
#if 1
    cpu_set_t _mask;
    CPU_ZERO(&_mask);
    CPU_SET(g_numa[thread_id], &_mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(_mask), &_mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }
#endif
    erpc::Nexus* _nexus = context->nexus;
    printf("CreateRPC - %d\n", _thread_id);
    context->rpc = new erpc::Rpc<erpc::CTransport>(_nexus, (void*)context, _thread_id, sm_handler);
    assert(context->rpc != nullptr);
    context->rpc->run_event_loop(1000000);
}

int main()
{
    hikv::Options _options;
    _options.pmem_file_size = 100UL * (1024 * 1024 * 1024);
    _options.index_size = 32UL * (1024 * 1024 * 1024);
    _options.store_size = 64UL * (1024 * 1024 * 1024);
    _options.num_server_threads = kNumServerThread;
    _options.num_backend_threads = 1;
    strcpy(_options.pmem_file_path, "/home/pmem/hikv");
    hikv::HiKV* _hikv = new hikv::HiKV(_options);

    std::thread _thread[128];
    std::string _server_uri = kServerHostname + ":" + std::to_string(kUDPPort);

    erpc::Nexus* _nexus = new erpc::Nexus(_server_uri, 0, 0);
    _nexus->register_req_func(kInsertType, req_insert_handle);
    _nexus->register_req_func(kSearchType, req_search_handle);

    for (int i = 0; i < kNumServerThread; i++) {
        ServerContext* __context = new ServerContext();
        __context->nexus = _nexus;
        __context->thread_id = i;
        __context->hikv = _hikv;
        _thread[i] = std::thread(run_server_thread, __context);
    }
    for (int i = 0; i < kNumServerThread; i++) {
        _thread[i].join();
    }
    return 0;
}
