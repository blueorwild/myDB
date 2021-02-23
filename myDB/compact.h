// ѹ������ÿ����Ծ��skipList��С�ﵽ��ֵ�󣬾���Ҫ����ѹ��������ͬʱ���ֻ����
// ����ÿ��skiplist��kv���ݡ�������ڴ�顢���ɵĹ�����
// ���Ǽ���ѹ�����Ľڵ����ݣ��ŵ�һ�������������ջ��

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

	// ����ѹ�����Ľӿڣ���3��������ǰ������ִ�е�д������������ҪΪ0����ִ�������������������ȴ�
	void DoCompact(Memory* memory, SkipList* skiplist, std::atomic_int* semaphore);

	// ѹ��׼������������������ѹ����skiplist������Ծ������ݲ�͸��������4��������DoCompact���ɵ�
	// ѹ�����ڵ㣬��Ϊ��������Ҫ�������֮��������ɣ�Ȼ��ӵ��ýڵ��У���Ҫ����ڵ��λ����Ϣ
	void CompactPrep(SkipList* skiplist, std::atomic_int* semaphore, Node* pNode);

	// ��ѹ�������ж�kv��������������Ϊ������Ч��
	void Query(const char* internalKey, std::string* value, SkipList* skipList);

	// ��ѹ���������ж�ĳkey�Ƿ���ڣ�������������Ϊ������Ч��
	bool Contain(const char* key, uint64_t s, SkipList* skipList);
};
#endif // !_MY_DB_COMPACT_H_
