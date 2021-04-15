
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence

correctness: SkipList.o SSTable.o kvstore.o correctness.o

persistence: kvstore.o persistence.o

clean:
	-rm -f correctness persistence *.o
