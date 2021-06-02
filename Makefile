LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence

correctness: SkipList.o SSTable.o kvstore.o correctness.o

persistence: SkipList.o SSTable.o kvstore.o persistence.o

delay: SkipList.o SSTable.o kvstore.o delay.o

compaction: SkipList.o SSTable.o kvstore.o compaction.o

clean:
	-rm -f correctness persistence delay compaction *.o
	-rm -rf data/*
	-rm -rf peekSSTable out.txt