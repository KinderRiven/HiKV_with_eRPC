/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-23 10:55:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /code/eRPC/hello_world/server.cc
 */
#include "common.h"
#include <thread>

struct ReqContext {
    uint32_t id;
};

erpc::Rpc<erpc::CTransport>* rpc;

ReqContext req_context;

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

void req_handler(erpc::ReqHandle* req_handle, void*)
{
    printf("req_handler begin\n");
    auto& resp = req_handle->pre_resp_msgbuf;
    rpc->resize_msg_buffer(&resp, kMsgSize);
    sprintf(reinterpret_cast<char*>(resp.buf), "hello");
    rpc->enqueue_response(req_handle, &resp);
    printf("req_handler end\n");
}

static void run_thread(erpc::Nexus* nexus)
{
    printf("new rpc\n");
    rpc = new erpc::Rpc<erpc::CTransport>(&nexus, static_cast<void*>(&req_context), 0, sm_handler);

    printf("run event loop\n");
    rpc->run_event_loop(10000000);
}

int main()
{
    std::thread* thread;
    std::string server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
    printf("%s\n", server_uri.c_str());

    printf("new nexus\n");
    erpc::Nexus nexus(server_uri, 0, 0);

    printf("register_req_func\n");
    nexus.register_req_func(kReqType, req_handler);

    thread = std::thread(run_thread, &nexus);
    thread->join();
}
