/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-26 11:10:14
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /code/eRPC/hello_world/server.cc
 */

#include "common.h"
#include "hikv.hpp"
#include <thread>

using namespace hikv;

struct ServerContext {
public:
    int thread_id;
    HiKV* hikv;
    erpc::Rpc<erpc::CTransport>* rpc = nullptr;
};

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

void req_insert_handle(erpc::ReqHandle* req_handle, void* context)
{
    printf("HandleInsertRequest\n");
    auto _req = req_handle->get_req_msgbuf();

    ServerContext* _context = (ServerContext*)context;
    // req_handle->dyn_resp_msgbuf = _context->rpc->alloc_msg_buffer_or_die(kMsgSize);
    // _context->rpc->resize_msg_buffer(&req_handle->dyn_resp_msgbuf, kMsgSize);
    // strcpy((char*)req_handle->dyn_resp_msgbuf.buf, "hello");
    // _context->rpc->enqueue_response(req_handle, &req_handle->dyn_resp_msgbuf);

    char* _buf = (char*)_req->buf;
    uint32_t _num_kv = *(uint32_t*)_buf;
    printf("%d\n", _num_kv);

    // Respnse
    auto& resp = req_handle->pre_resp_msgbuf;
    _context->rpc->resize_msg_buffer(&resp, kMsgSize);
    sprintf(reinterpret_cast<char*>(resp.buf), "hello");
    _context->rpc->enqueue_response(req_handle, &resp);
}

void req_search_handle(erpc::ReqHandle* req_handle, void* context)
{
    // Receive
    printf("HandleSearchRequest\n");
    auto _req = req_handle->get_req_msgbuf();

    // Respnse
    ServerContext* _context = (ServerContext*)context;
    auto& resp = req_handle->pre_resp_msgbuf;
    _context->rpc->resize_msg_buffer(&resp, kMsgSize);
    sprintf(reinterpret_cast<char*>(resp.buf), "hello");
    _context->rpc->enqueue_response(req_handle, &resp);
}

static void run_server_thread(ServerContext* context)
{
    erpc::Rpc<erpc::CTransport>* _rpc = context->rpc;
    assert(_rpc != nullptr);
    _rpc->run_event_loop(1000000);
}

int main()
{
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Options _options;
    _options.pmem_file_size = 100UL * (1024 * 1024 * 1024);
    _options.index_size = 32UL * (1024 * 1024 * 1024);
    _options.store_size = 64UL * (1024 * 1024 * 1024);
    _options.num_server_threads = 16;
    _options.num_backend_threads = 4;
    _num_kv /= _options.num_server_threads;
    strcpy(_options.pmem_file_path, "/home/pmem2/hikv");
    HiKV* _hikv = new HiKV(_options);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    std::thread _thread[128];
    std::string _server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    erpc::Nexus _nexus(_server_uri, 0, 0);
    _nexus.register_req_func(kInsertType, req_insert_handle);
    _nexus.register_req_func(kSearchType, req_search_handle);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < kNumServerThread; i++) {
        ServerContext* __context = new ServerContext();
        erpc::Rpc<erpc::CTransport>* __rpc = new erpc::Rpc<erpc::CTransport>(&_nexus, static_cast<void*>(__context), i, sm_handler);
        __context->thread_id = i;
        __context->rpc = __rpc;
        __context->hikv = _hikv;
        _thread[i] = std::thread(run_server_thread, __context);
    }
    for (int i = 0; i < kNumServerThread; i++) {
        _thread[i].join();
    }
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}
