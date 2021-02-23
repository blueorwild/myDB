// 压缩器，每当活跃的skipList大小达到阈值后，就需要交给压缩器清理同时变成只读的
// 这里每张skiplist有kv内容、分配的内存块、生成的过滤器
// 它们即是压缩器的节点内容，放到一个单向链表里（链栈）

#ifndef _MY_DB_COMPACT_H_
#define _MY_DB_COMPACT_H_

#include <atomic>
#include <string>

#include "memory.h"
#include "skiplist.h"
#include "snapshot.h"
#include "bloom.h"

class Compact {
private:
	struct Node{
		Memory* memory_;
		SkipList* skiplist_;
		Bloom* bloom_;
		Node* next_;
	};

	Node* head_;

public:
	Compact();
	~Compact();

	// 进入压缩器的接口，第3个参数当前表正在执行的写操作数量，需要为0才能执行清理工作，否则阻塞等待
	void DoCompact(Memory* memory, SkipList* skiplist, std::atomic_int* semaphore);

	// 压缩准备工作（真正的数据压缩在skiplist，这里对具体数据不透明）；第4个参数是DoCompact生成的
	// 压缩器节点，因为过滤器需要清理完成之后才能生成，然后加到该节点中，需要保存节点的位置信息
	void CompactPrep(SkipList* skiplist, std::atomic_int* semaphore, Node* pNode);

	// 在压缩器链中读kv，第三个参数是为了提升效率
	void Query(const char* internalKey, std::string* value, SkipList* skipList);

	// 在压缩器链中判断某key是否存在，第三个参数是为了提升效率
	bool Contain(const char* key, uint64_t s, SkipList* skipList);
};
#endif // !_MY_DB_COMPACT_H_
