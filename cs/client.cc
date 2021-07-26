/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-26 13:35:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/benchmark/cs/client.cc
 */
#include "common.h"

struct ClientContext {
public:
    int thread_id;
    std::string client_uri;
    std::string server_uri;

    erpc::Nexus* nexus;
    erpc::Rpc<erpc::CTransport>* rpc;
    erpc::MsgBuffer req;
    erpc::MsgBuffer resp;
};

void cont_func(void* context, void* tag)
{
    printf("cont_func.\n");
}

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

static void run_client_thread(ClientContext* context)
{
    int _thread_id = context->thread_id;
    erpc::Nexus* _nexus = context->nexus;
    context->rpc = new erpc::Rpc<erpc::CTransport>(&_nexus, nullptr, _thread_id, sm_handler);

    int _session_num = rpc->create_session(context->server_uri, _thread_id % kNumServerThread);
    printf("[%d][Session:%d]\n", _thread_id, _session_num);

    while (!context->rpc->is_connected(_session_num)) {
        context->rpc->run_event_loop_once();
    }
    printf("[%d][Connect Finished]\n", _thread_id);

    context->req = rpc->alloc_msg_buffer_or_die(kMsgSize);
    context->resp = rpc->alloc_msg_buffer_or_die(kMsgSize);
    // rpc->enqueue_request(session_num, kInsertType, &req, &resp, cont_func, nullptr);
    context->rpc->run_event_loop(1000000);
}

int main()
{
    std::string _client_uri = kClientHostname + ":" + std::to_string(kUDPPort);
    printf("ClientHostName:%s\n", _client_uri.c_str());

    erpc::Nexus* _nexus = new erpc::Nexus(client_uri, 0, 0);
    rpc = new erpc::Rpc<erpc::CTransport>(&nexus, nullptr, 0, sm_handler);

    std::thread _thread[128];
    std::string _server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
    printf("ServerHostName:%s\n", _server_uri.c_str());

    for (int i = 0; i < kNumClientThread; i++) {
        ClientContext* __context = new ClientContext();
        __context->thread_id = i;
        __context->nexus = _nexus;
        __context->client_uri = _client_uri;
        __context->server_uri = _server_uri;
        _thread[i] = std::thread(run_client_thread, __context);
    }
    for (int i = 0; i < kNumClientThread; i++) {
        _thread[i].join();
    }
}
