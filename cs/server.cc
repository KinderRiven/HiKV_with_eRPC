/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-23 14:45:20
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /code/eRPC/hello_world/server.cc
 */

#include "common.h"
#include <thread>

struct ServerContext {
public:
    int thread_id;
    erpc::Rpc<erpc::CTransport>* rpc = nullptr;
};

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

void req_insert_handle(erpc::ReqHandle* req_handle, void* context)
{
    printf("req_insert_handle\n");
    ServerContext* _context = (ServerContext*)context;
    auto& resp = req_handle->pre_resp_msgbuf;
    context->rpc->resize_msg_buffer(&resp, kMsgSize);
    sprintf(reinterpret_cast<char*>(resp.buf), "hello");
    context->rpc->enqueue_response(req_handle, &resp);
}

void req_search_search(erpc::ReqHandle* req_handle, void* context)
{
    printf("req_search_search\n");
    ServerContext* _context = (ServerContext*)context;
    auto& resp = req_handle->pre_resp_msgbuf;
    context->rpc->resize_msg_buffer(&resp, kMsgSize);
    sprintf(reinterpret_cast<char*>(resp.buf), "hello");
    context->rpc->enqueue_response(req_handle, &resp);
}

static void run_server_thread(ServerContext* context)
{
    erpc::Rpc<erpc::CTransport>* _rpc = context->rpc;
    assert(_rpc != nullptr);
    _rpc->run_event_loop(10000000);
}

int main()
{
    std::thread _thread[128];
    std::string _server_uri = kServerHostname + ":" + std::to_string(kUDPPort);

    erpc::Nexus _nexus(_server_uri, 0, 0);
    _nexus.register_req_func(kInsertType, req_insert_handle);
    _nexus.register_req_func(kSearchType, req_search_handle);

    for (int i = 0; i < kNumServerThread; i++) {
        ServerContext* __context = new ServerContext();
        erpc::Rpc<erpc::CTransport>* __rpc = new erpc::Rpc<erpc::CTransport>(_nexus, static_cast<void*>(__context), i, sm_handler);
        __context->thread_id = i;
        __context->rpc = __rpc;
        _thread[i] = std::thread(run_server_thread, __context);
    }
    for (int i = 0; i < kNumServerThread; i++) {
        _thread[i].join();
    }
}