/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-25 16:56:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/benchmark/cs/client.cc
 */
#include "common.h"

erpc::Rpc<erpc::CTransport>* rpc;
erpc::MsgBuffer req;
erpc::MsgBuffer resp;

void cont_func(void*, void*)
{
    printf("cont_func\n");
}

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

int main()
{
    std::string client_uri = kClientHostname + ":" + std::to_string(kUDPPort);
    erpc::Nexus nexus(client_uri, 0, 0);
    printf("%s\n", client_uri.c_str());

    rpc = new erpc::Rpc<erpc::CTransport>(&nexus, nullptr, 0, sm_handler);

    std::string server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
    printf("%s\n", server_uri.c_str());

    int session_num = rpc->create_session(server_uri, 0);
    printf("create_session-%d.\n", session_num);

    while (!rpc->is_connected(session_num)) {
        rpc->run_event_loop_once();
    }
    printf("connect finished.\n");

    req = rpc->alloc_msg_buffer_or_die(kMsgSize);
    resp = rpc->alloc_msg_buffer_or_die(kMsgSize);

    // rpc->resize_msg_buffer(&req, kMsgSize);
    uint64_t* _src = (uint64_t*)req.buf;
    *_src = 0x12345678;

    rpc->enqueue_request(session_num, kReqType, &req, &resp, cont_func, nullptr);
    rpc->run_event_loop(100);
    delete rpc;
}
