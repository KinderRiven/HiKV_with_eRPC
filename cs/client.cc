/*
 * @Author: your name
 * @Date: 2021-04-08 10:36:18
 * @LastEditTime: 2021-07-27 12:34:31
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /HiKV+++/benchmark/cs/client.cc
 */
#include "common.h"

#define REQ_INSERT (1)
#define REQ_SEARCH (2)

struct RequestContext {
public:
    int type;
    int complete;
    size_t start_time;
    size_t end_time;
    erpc::MsgBuffer req;
    erpc::MsgBuffer resp;
};

struct ResultContext {
public:
    uint64_t num_insert_ok;
    uint64_t num_insert_error;
    uint64_t num_search_ok;
    uint64_t num_search_error;
};

struct ClientContext {
public:
    int thread_id;
    uint64_t base;
    uint64_t num_kv;

    std::string client_uri;
    std::string server_uri;

    erpc::Nexus* nexus;
    erpc::Rpc<erpc::CTransport>* rpc;

    RequestContext request;
    ResultContext result;

public:
    ClientContext()
        : rpc(nullptr)
    {
        memset(&result, 0, sizeof(result));
        memset(&request, 0, sizeof(request));
    }
};

void sm_handler(int, erpc::SmEventType, erpc::SmErrType, void*) { }

void kv_cont_func(void* context, void* tag)
{
    ClientContext* _context = (ClientContext*)context;
    _context->request.end_time = erpc::rdtsc();
    if (_context->request.type == REQ_INSERT) {
        _context->result.num_insert_ok++;
        _context->request.complete = 1;
    } else if (_context->request.type == REQ_SEARCH) {
        char* _buf = (char*)_context->request.resp.buf;
        uint64_t _num_kv = *(uint64_t*)_buf;
        // printf("[%llu]\n", _num_kv);
        _buf += kHeadSize;
        for (int i = 0; i < _num_kv; i++) {
            uint64_t _key = *(uint64_t*)_buf;
            _buf += kKeySize;
            uint64_t _value = *(uint64_t*)_buf;
            _buf += kValueSize;
            if (_key == _value) {
                _context->result.num_search_ok++;
            } else {
                _context->result.num_search_error++;
            }
        }
        _context->request.complete = 1;
    }
}

static void run_client_thread(ClientContext* context)
{
    int _thread_id = context->thread_id;
    erpc::Nexus* _nexus = context->nexus;

    context->rpc = new erpc::Rpc<erpc::CTransport>(_nexus, (void*)context, _thread_id, sm_handler);
    erpc::Rpc<erpc::CTransport>* _rpc = context->rpc;
    int _session_num = _rpc->create_session(context->server_uri, _thread_id % kNumServerThread);
    printf("[%d][Session:%d]\n", _thread_id, _session_num);

    while (!_rpc->is_connected(_session_num)) {
        _rpc->run_event_loop_once();
    }
    printf("[%d][Connect Finished]\n", _thread_id);
    context->request.req = _rpc->alloc_msg_buffer_or_die(kMsgSize * kNumBatch);
    context->request.resp = _rpc->alloc_msg_buffer_or_die(kMsgSize * kNumBatch);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    uint64_t _base = context->base;
    size_t _start_time = erpc::rdtsc();
    for (uint64_t i = 1; i <= context->num_kv; i += kNumBatch) {
        context->request.type = REQ_INSERT;
        context->request.complete = 0;
        // +++++++++++++++++++++++++++++++++++++++
        char* __dest = (char*)context->request.req.buf;
        *(uint64_t*)__dest = kNumBatch;
        __dest += kHeadSize;
        for (int j = 0; j < kNumBatch; j++) {
            *(uint64_t*)__dest = _base;
            __dest += kKeySize;
            *(uint64_t*)__dest = _base;
            __dest += kValueSize;
            _base++;
        }
        // +++++++++++++++++++++++++++++++++++++++
        context->request.start_time = erpc::rdtsc();
        _rpc->enqueue_request(_session_num, kInsertType, &context->request.req, &context->request.resp, kv_cont_func, nullptr);
        while (!context->request.complete) {
            _rpc->run_event_loop_once();
        }
        // +++++++++++++++++++++++++++++++++++++++
    }
    size_t _end_time = erpc::rdtsc();
    double _lat = erpc::to_usec(_end_time - _start_time, _rpc->get_freq_ghz());
    printf("[insert][%d][%llu][time:%.2fseconds][iops:%.2f]\n",
        _thread_id, context->result.num_insert_ok,
        _lat / 1000000.0, 1.0 * context->result.num_insert_ok / (_lat / 1000000.0));
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    _rpc->resize_msg_buffer(&context->request.req, (kHeadSize + kKeySize) * kNumBatch);
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    _base = context->base;
    _start_time = erpc::rdtsc();
    for (uint64_t i = 1; i <= context->num_kv; i += kNumBatch) {
        context->request.type = REQ_SEARCH;
        context->request.complete = 0;
        // +++++++++++++++++++++++++++++++++++++++
        char* __dest = (char*)context->request.req.buf;
        *(uint64_t*)__dest = kNumBatch;
        __dest += kHeadSize;
        for (int j = 0; j < kNumBatch; j++) {
            *(uint64_t*)__dest = _base;
            __dest += kKeySize;
            _base++;
        }
        // +++++++++++++++++++++++++++++++++++++++
        context->request.start_time = erpc::rdtsc();
        _rpc->enqueue_request(_session_num, kSearchType, &context->request.req, &context->request.resp, kv_cont_func, nullptr);
        while (!context->request.complete) {
            _rpc->run_event_loop_once();
        }
        // +++++++++++++++++++++++++++++++++++++++
    }
    _end_time = erpc::rdtsc();
    _lat = erpc::to_usec(_end_time - _start_time, _rpc->get_freq_ghz());
    printf("[search][%d][%llu/%llu][time:%.2fseconds][iops:%.2f]\n",
        _thread_id, context->result.num_insert_ok, context->result.num_insert_error,
        _lat / 1000000.0, 1.0 * context->result.num_insert_ok / (_lat / 1000000.0));
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

int main()
{
    std::string _client_uri = kClientHostname + ":" + std::to_string(kUDPPort);
    erpc::Nexus* _nexus = new erpc::Nexus(_client_uri, 0, 0);
    printf("ClientHostName:%s\n", _client_uri.c_str());

    std::thread _thread[128];
    std::string _server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
    printf("ServerHostName:%s\n", _server_uri.c_str());

    for (int i = 0; i < kNumClientThread; i++) {
        ClientContext* __context = new ClientContext();
        __context->thread_id = i;
        __context->nexus = _nexus;
        __context->client_uri = _client_uri;
        __context->server_uri = _server_uri;
        __context->base = i * (kNumOpt / kNumClientThread);
        __context->num_kv = (kNumOpt / kNumClientThread);
        _thread[i] = std::thread(run_client_thread, __context);
    }
    for (int i = 0; i < kNumClientThread; i++) {
        _thread[i].join();
    }
}
