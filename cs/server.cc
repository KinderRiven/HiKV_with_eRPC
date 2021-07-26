/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-26 12:03:36
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
    int thread_id = context->thread_id;
    erpc::Nexus* _nexus = context->nexus;
    printf("CreateRPC - %d\n", thread_id);
    context->rpc = new erpc::Rpc<erpc::CTransport>(&_nexus, (void*)context, thread_id, sm_handler);
    assert(context->rpc != nullptr);
    context->rpc->run_event_loop(1000000);
}

int main()
{
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    hikv::Options _options;
    _options.pmem_file_size = 100UL * (1024 * 1024 * 1024);
    _options.index_size = 1UL * (1024 * 1024 * 1024);
    _options.store_size = 8UL * (1024 * 1024 * 1024);
    _options.num_server_threads = kNumServerThread;
    _options.num_backend_threads = 1;
    strcpy(_options.pmem_file_path, "/home/pmem/hikv");
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
        __context->nexus = _nexus;
        __context->thread_id = i;
        __context->hikv = _hikv;
        _thread[i] = std::thread(run_server_thread, __context);
    }
    for (int i = 0; i < kNumServerThread; i++) {
        _thread[i].join();
    }
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}
