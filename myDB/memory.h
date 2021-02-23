// 内存分配器，是一条链，每个节点是一块一次分配的大内存，当需要分配内存时
// 先从这些已分配好的大内存块中尝试取得内存，如果失败的话再尝试整块的分配大内存
// 有效的避免了大量内存碎片的出现以及频繁小内存分配浪费时间，提高了内存管理效率
// 另外考虑到AtomicPointer类的要求，此封装也实现了内存对齐的分配。

#ifndef _MY_DB_MEMORY_H_
#define _MY_DB_MEMORY_H_

#include <vector>
#include "my_lock.h"

class Memory {
private:
	std::vector<char*> blocks_;     // 存放若干指针，每个指针指向一块内存
	char* unusePtr_;                // 指向当前内存块未使用空间的首址
	size_t unuseSize_;              // 当前内存块未使用空间的大小
	size_t alloctedSize_;           // 记录到目前为止已分配的内存总大小

	static const int blockSize = 4096;       // 4KB,一个比较合适的内存块大小
	SpinLock alloclock_;            // 用于内存分配的自旋锁

	// 当前内存块剩余空间不足的处理方法
	char* AllocateFallback(size_t bytes);
	// 分配新的内存块
	char* AllocateNewBlock(size_t bytes);

public:
	Memory();
	~Memory();

	// 普通分配：返回一个指向bytes大小的内存块地址首址的指针
	char* Allocate(size_t bytes);

	// 保证内存对齐的分配(首地址对齐)
	char* AllocateAligned(size_t bytes);

	// 返回分配的总内存【已使用】量(不包括已分配但尚未使用的空间)。
	size_t MemoryUsage();

};

#endif  // !_MY_DB_MEMORY_H_
