/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-23 10:47:44
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /code/eRPC/hello_world/server.cc
 */
#include "common.h"

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

int main()
{
    std::string server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
    printf("%s\n", server_uri.c_str());

    printf("new nexus\n");
    erpc::Nexus nexus(server_uri, 0, 0);

    printf("register_req_func\n");
    nexus.register_req_func(kReqType, req_handler);

    printf("new rpc\n");
    rpc = new erpc::Rpc<erpc::CTransport>(&nexus, &req_context, 0, sm_handler);

    printf("run event loop\n");
    rpc->run_event_loop(10000000);
    printf("finished\n");
}
