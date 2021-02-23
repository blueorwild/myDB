#ifndef _MY_DB_IMPL_H_
#define _MY_DB_IMPL_H_

#include <atomic>
#include <string>

#include "db.h"
#include "skiplist.h"
#include "snapshot.h"
#include "seq_num.h"
#include "memory.h"
#include "my_lock.h"
#include "compact.h"


class DBImpl:public DB {
private:
	AtomicPointer head_;       // 当前活跃SkipList的头结点
	AtomicPointer memory_;     // 统一的内存分配管理，详见对应文件
	SequenceNumber* SN_;       // 全局序列号，详见对应文件
	Compact *compact_;         // 一个数据压缩器，用于清理达到阈值的SkipList

	static const int maxSize_ = 1<<22;   // 4MB,SkipList大小的阈值

	SpinLock compactlock_;     // 用于数据压缩的自旋锁
	std::atomic_int* semaphore_;  // 代表当前活跃表正在进行写操作的线程数
	std::mutex hello;

public:
	DBImpl();
	~DBImpl();

	// **************以下是db接口的具体实现*************
	void DeleteDB();

    void Add(const char* key, const char* value);
	void Delete(const char* key);
	void Query(const SnapShot* snapShot, const char* key, std::string* value);
	void Alter(const char* key, const char* newValue);

	const SnapShot* GetSnapShot();
	void DeleteSnapShot(const SnapShot* snapshot);

	// **************以上是db接口的具体实现*************
	
	// **************以下是db接口实现所需的一些功能函数*************
	// 当前是否需要数据清理（数据清理会被写阻塞，不会阻塞读写）
	void NeedCompact(int size);
};

#endif // !_MY_DB_H_