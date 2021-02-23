#ifndef _MY_DB_SKIPLIST_H_
#define _MY_DB_SKIPLIST_H_

#include <string>
#include <assert.h>

#include "memory.h"
#include "my_lock.h"
#include "random.h"
#include "bloom.h"

class SkipList {

private: 
	static const int MAX_LEVEL = 12;   // 跳表限制最大高度（0~11）
	int maxLevel_;                     // 跳表实际最大高度

	// 跳表节点结构体
	struct Node {            
	public:
		char* data;     // 节点的值(所有内容拼接成一个字符串)

		// 原子性的读
		Node* AtomicNext(int n) {
			return (Node*)(next_[n].AtomicLoad());
		}
		// 原子性的写
		void AtomicSetNext(int n, Node* x) {
			next_[n].AtomicStore(x);
		}
		// 普通读
		Node* Next(int n) {
			return (Node*)(next_[n].Load());
		}
		// 普通写
		void SetNext(int n, Node* x) {
			this->next_[n].Store(x);
		}

	private:
		// 跳表节点的指针数组,只能放在后面，因为这个指针数组需要动态增长
		// 而数组的空间是连续的，如果放在data的前面，增长的时候会覆盖掉data
		AtomicPointer next_[1];     
	
	};
	Node* head_;              // 跳表头节点

	Memory *memory_;          // 统一的内存分配，来自上层的传递
	Random *random_;          // 随机层数生成器，详见对应文件
	MutexLock  mutex_;         // 互斥锁(用于Write操作)

public:
	SkipList(Memory *memory);
	~SkipList();

	// *****************以下供上层调用********************
	// 判断SkipList中是否存在对应序列号小于s的key的记录
	// 返回值 -1 代表肯定不存在
	//        0 代表当前表不存在
	//        1 代表已经找到
	int Contain(const char* key, uint64_t s);

	// 根据internalKey从SkipList中读取对应的value值，若不存在返回false
	// internalKey由用户key，序列号和记录类型（新增/删除）拼接而成
	// 返回值 -1 代表肯定不存在
	//        0 代表当前表不存在
	//        1 代表已经找到
	int Read(const char* internalKey, std::string* value);

	// 往SkipList中插入数据
	// 返回值 -1 插入失败，存在重复数据
	//        0 插入失败，发生会导致错误的并发冲突
	//        1 插入成功
	int Write(char* data);

	// 数据清理，具体方法是对 序列号<=s的记录 只保留序列号最大的那个，
	// 如果序列号最大的那个类型为删除，则也一并清理掉
	void DataMerge(uint64_t s);

	// 把当前skiplist的key放进传进来的bloom过滤器中
	void AddBloomFilter(Bloom* bloom);
	// *****************以上供其它调用********************

	// *****************以下供自身使用********************
	// a、b为Node中的data，以下通用，
	// 判断data的Key和Key(用户key)的大小关系，由小到大排序规则按Key字典升序
	int KeyCompare(const char* data, const char* key);

	// 判断a的Key和b的Key是否相等(用户key)
	bool KeyIsEqual(const char* a, const char* b);

	// 判断data的InternalKey是否更小，由小到大排序规则按key字典升序, 序列号的降序
	bool InternalKeyIsLess(const char* data, const char* key, uint64_t s);

	// 判断a、b的value是否相同（不能直接用strcmp比较，因为没有结束符）
	bool ValueIsEqual(const char* a, const char* b);

	// 用于数据清理，按顺序找到某层中第一个需要清理的节点，
	// p用于保存需要清理节点的前置节点，level为层数
	bool FindNeedClear(Node* &p, uint64_t s, int level);

	// 用于数据清理，p为需要清理的节点，level为层数
	// 功能是按顺序找到某层p后第一个不需要清理的节点，
	Node* FindNotNeedClear(Node* p, uint64_t s, int level);

	// 创建一个值为data，层数为level的新SkipList节点
	Node* NewNode(int level, char* data);
};

#endif // !_MY_DB_SKIPLIST_H_