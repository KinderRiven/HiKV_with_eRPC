#Notes:
#1. The make target depends on how eRPC was compiled:
#   * If DTRANSPORT=dpdk, use `make dpdk`
#   * If DTRANSPORT=raw, use `make raw`
#   * If DTRANSPORT=infiniband, use `make infiniband`
LIBS = -lerpc -lpthread -lnuma -ldl -mrtm 
INCLUDE_HEADER := -I../third-party/eRPC-master/src -I../hikv 
HIKV_LINK := -L../hikv/build -lhikv -L../third-party/pmdk -lpmem -L../third-party/tbb -ltbb
ERPC_LINK := -L../third-party/eRPC-master/build -libverbs -DERPC_INFINIBAND=true -DROCE=true 

all: client server

client:
	g++ -g -std=c++11 -o server server.cc $(INCLUDE_HEADER) $(ERPC_LINK) $(HIKV_LINK) $(LIBS)

server:
	g++ -g -std=c++11 -o client client.cc $(INCLUDE_HEADER) $(ERPC_LINK) $(HIKV_LINK) $(LIBS)

clean:
	rm server client

export:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../hikv/build:../third-party/pmdk:../third-party/tbb
