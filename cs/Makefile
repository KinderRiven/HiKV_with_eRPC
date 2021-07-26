#Notes:
#1. The make target depends on how eRPC was compiled:
#   * If DTRANSPORT=dpdk, use `make dpdk`
#   * If DTRANSPORT=raw, use `make raw`
#   * If DTRANSPORT=infiniband, use `make infiniband`
LIBS = -lerpc -lpthread -lnuma -ldl
INCLUDE_HEADER := -I ../third-party/eRPC-master/src -I ../hikv 
HIKV_LINK := -L../hikv/build -lhikv -L../third-party/pmdk -lpmem -L../third-party/tbb -ltbb
ERPC_LINK := -L ../third-party/eRPC-master/build -libverbs -DERPC_INFINIBAND=true -DROCE=true 

all:
	g++ -g -std=c++11 -o server server.cc $(INCLUDE_HEADER) $(ERPC_LINK) $(HIKV_LINK)
	g++ -g -std=c++11 -o client client.cc $(INCLUDE_HEADER) $(ERPC_LINK) $(HIKV_LINK)

clean:
	rm server client
