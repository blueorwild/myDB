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
	AtomicPointer head_;       // ��ǰ��ԾSkipList��ͷ���
	AtomicPointer memory_;     // ͳһ���ڴ������������Ӧ�ļ�
	SequenceNumber* SN_;       // ȫ�����кţ������Ӧ�ļ�
	Compact *compact_;         // һ������ѹ��������������ﵽ��ֵ��SkipList

	static const int maxSize_ = 1<<22;   // 4MB,SkipList��С����ֵ

	SpinLock compactlock_;     // ��������ѹ����������
	std::atomic_int* semaphore_;  // ����ǰ��Ծ�����ڽ���д�������߳���
	std::mutex hello;

public:
	DBImpl();
	~DBImpl();

	// **************������db�ӿڵľ���ʵ��*************
	void DeleteDB();

    void Add(const char* key, const char* value);
	void Delete(const char* key);
	void Query(const SnapShot* snapShot, const char* key, std::string* value);
	void Alter(const char* key, const char* newValue);

	const SnapShot* GetSnapShot();
	void DeleteSnapShot(const SnapShot* snapshot);

	// **************������db�ӿڵľ���ʵ��*************
	
	// **************������db�ӿ�ʵ�������һЩ���ܺ���*************
	// ��ǰ�Ƿ���Ҫ����������������ᱻд����������������д��
	void NeedCompact(int size);
};

#endif // !_MY_DB_H_