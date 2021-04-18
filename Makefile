
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence

correctness: BloomFilter.o SkipList.o SSTable.o kvstore.o correctness.o

persistence: kvstore.o persistence.o

try: BloomFilter.o SkipList.o SSTable.o kvstore.o try.o

clean:
	-rm -f correctness persistence try *.o
	-rm -rf data/*
