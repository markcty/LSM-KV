## Data structure project 2: KVStore using Log-structured Merge Tree

LSM Tree(Log-structure Merge Tree)数据结构，于 1996 年在 Patrick O’Neil 等人的一篇论文提出。其通过 SS-Table 的多层储存结构，利用磁盘顺序读写的高效性，实现了性能极高的写操作。LSM Tree 被广泛地在各种 NoSQL 中使用，比如 HBase，LevelDB 等。

LSM Tree 键值存储系统分为内存存储和硬盘存储两部分，内存部分使用跳表或平衡树（本项目使用跳表实现）。

硬盘存储采用分层存储的方式进行存储，每一层中包括多个文件，每个文件被称为 SSTable（Sorted Strings Table），用于有序地存储多个键值对（Key-Value Pairs）， 该项目中一个 SSTable 的大小为 2 MB 。

SSTable 是保存在磁盘中的，而磁盘的读写速度比内存要慢几个数量级。因此在查找时，去磁盘读取 SSTable 的 Bloom Filter 和索引是很耗时的操作。为了避免多次磁盘的读取操作，我们可以 将 SSTable 中除了数据区外的 其他部分缓存在内存中 。

当插入一个键值对触发伐值（内存中保存数据到达 2MB），会触发 compaction 操作，将内存中的数据写入硬盘，若当前层满了，则会递归地向更高层写数据。

具体细节见 pdf 文档，性能测试报告见 report 目录下的 latex。
