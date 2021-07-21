all:
	g++ -std=c++11 example.cc -o example -I../../hikv -L../../hikv/build -lhikv -L../../third-party/pmdk -lpmem -L../../third-party/tbb -ltbb -lpthread -mrtm 

export:
	export LD_LIBRARY_PATH=../../hikv/build:../../third-party/pmdk:../../third-party/tbb