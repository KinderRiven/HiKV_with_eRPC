/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-25 17:47:33
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
    auto _req = req_handle->get_req_msgbuf();
    printf("%llx\n", *(uint64_t*)_req->buf);

    ServerContext* _context = (ServerContext*)context;
    _context->rpc->resize_msg_buffer(&req_handle->dyn_resp_msgbuf, kMsgSize);
    strcpy((void*)req_handle->dyn_resp_msgbuf.buf, "hello");
    _context->rpc->enqueue_response(req_handle, &req_handle->dyn_resp_msgbuf);
}

void req_search_handle(erpc::ReqHandle* req_handle, void* context)
{
    // Receive
    printf("req_search_search\n");
    auto _req = req_handle->get_req_msgbuf();
    printf("%llx\n", *(uint64_t*)_req->buf);

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
        erpc::Rpc<erpc::CTransport>* __rpc = new erpc::Rpc<erpc::CTransport>(&_nexus, static_cast<void*>(__context), i, sm_handler);
        __context->thread_id = i;
        __context->rpc = __rpc;
        _thread[i] = std::thread(run_server_thread, __context);
    }
    for (int i = 0; i < kNumServerThread; i++) {
        _thread[i].join();
    }
}
