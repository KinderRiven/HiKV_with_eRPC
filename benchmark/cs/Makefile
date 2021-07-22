#Notes:
#1. The make target depends on how eRPC was compiled:
#   * If DTRANSPORT=dpdk, use `make dpdk`
#   * If DTRANSPORT=raw, use `make raw`
#   * If DTRANSPORT=infiniband, use `make infiniband`
LIBS = -lerpc -lpthread -lnuma -ldl

all:
	g++ -g -std=c++11 -o server server.cc -I ../../third-party/eRPC-master/src -L ../../third-party/eRPC-master/build $(LIBS) -libverbs -DERPC_INFINIBAND=true -DROCE=true 
	g++ -g -std=c++11 -o client client.cc -I ../../third-party/eRPC-master/src -L ../../third-party/eRPC-master/build $(LIBS) -libverbs -DERPC_INFINIBAND=true -DROCE=true 

clean:
	rm server client

