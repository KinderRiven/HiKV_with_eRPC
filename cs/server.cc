/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-26 19:33:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /code/eRPC/hello_world/server.cc
 */

#include "common.h"
#include "hikv.hpp"
#include <thread>

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
    uint64_t _num_kv = *(uint64_t*)_buf;
    _buf += kHeadSize;

    uint64_t _key = *(uint64_t*)_buf;
    char* _skey = (char*)_buf;

    _buf += kKeySize;
    uint64_t _value = *(uint64_t*)_buf;
    char* _svalue = (char*)_buf;

    hikv::HiKV* _hikv = _context->hikv;
    bool _res = _hikv->Put(_context->thread_id, _skey, kKeySize, _svalue, kValueSize);

    // Respnse
    auto& resp = req_handle->pre_resp_msgbuf;
    _context->rpc->resize_msg_buffer(&resp, 8);
    if (_res) {
        *(uint64_t*)resp.buf = 1;
    } else {
        *(uint64_t*)resp.buf = 2;
    }
    _context->rpc->enqueue_response(req_handle, &resp);
}

void req_search_handle(erpc::ReqHandle* req_handle, void* context)
{
    ServerContext* _context = (ServerContext*)context;
    hikv::HiKV* _hikv = _context->hikv;
    // ++++++++++++++++++++++++++++++++++++++
    auto _req = req_handle->get_req_msgbuf();
    char* _buf = (char*)_req->buf;
    uint64_t _num_kv = *(uint64_t*)_buf;
    _buf += kHeadSize;
    uint64_t _key = *(uint64_t*)_buf;
    char* _skey = (char*)_buf;
    // ++++++++++++++++++++++++++++++++++++++
    auto& resp = req_handle->pre_resp_msgbuf;
    _context->rpc->resize_msg_buffer(&resp, kHeadSize + kKeySize + kValueSize);
    // ++++++++++++++++++++++++++++++++++++++
    char* _resp_buf = (char*)resp.buf;
    *(uint64_t*)_resp_buf = 1;
    _resp_buf += kHeadSize;
    *(uint64_t*)_resp_buf = _key;
    _resp_buf += kKeySize;

    char* _hikv_value = nullptr;
    size_t _hikv_value_length;
    bool _res = _hikv->Get(_context->thread_id, _skey, kKeySize, &_hikv_value, _hikv_value_length);
    memcpy(_resp_buf, _hikv_value, _hikv_value_length);
    _context->rpc->enqueue_response(req_handle, &resp);
    delete _hikv_value;
}

static void run_server_thread(ServerContext* context)
{
    int _thread_id = context->thread_id;
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
    _options.index_size = 1UL * (1024 * 1024 * 1024);
    _options.store_size = 8UL * (1024 * 1024 * 1024);
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
