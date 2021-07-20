all:
	g++ -std=c++11 example.cc -o example -I../../hikv -L../../hikv/build -lhikv -L../../third-party/pmdk -lpmem -lpthread

export:
	export LD_LIBRARY_PATH=../../hikv/build:../../third-party/pmdk