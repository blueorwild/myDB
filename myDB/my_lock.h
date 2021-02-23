// �˴����������й����Ĳ���

#ifndef _MY_DB_MY_LOCK_H_
#define _MY_DB_MY_LOCK_H_

#include <windows.h>
#include <atomic>
#include <mutex>

// ������
class SpinLock{
private:
	long int flag_;

public:
	SpinLock() { 
		// flag_��Ϊ��0��
		flag_ = 0;
	}

	void lock(){
		while (InterlockedExchange(&flag_, 1)) Sleep(0);
	}

	void unlock(){
		InterlockedExchange(&flag_, 0);
	}
};

// ������2
class SpinLock2 {
private:
	std::atomic_flag flag_;

public:
	SpinLock2() {
		// flag_��Ϊ��0��
		flag_.clear();
	}

	void lock() {
		while (flag_.test_and_set()) Sleep(0);
	}

	void unlock() {
		flag_.clear();
	}
};

// ��������ֱ���ÿ��ļ��ģ�ûʲô��˵�ģ�
class MutexLock {
private:
	std::mutex mutex_;

public:
	MutexLock() {};
	~MutexLock() {};

	void lock() {
		mutex_.lock();
	}

	void unlock() {
		mutex_.unlock();
	}
};

// ����ԭ�Ӳ���(ԭ��ָ��)
class AtomicPointer {
private:
	void * ptr_;
public:
	AtomicPointer() : ptr_(NULL) {}

	// Interlocked�����Ĺ���ԭ��ȡ���ڴ������е�CPUƽ̨�������x86ϵ��CPU��
	// ��ôInterlocked��������������ά��һ��Ӳ���źţ�����źŻ���ֹ����CPU
	// ����ͬһ���ڴ��ַ���������ȷ��������Щ�����ı�����ַ�Ǿ��������

	void* AtomicLoad() const {
		void * p = NULL;
		InterlockedExchangePointer(&p, ptr_);
		return p;
	}

	void AtomicStore(void* v) {
		InterlockedExchangePointer(&ptr_, v);
	}

	void* Load() const {
		return ptr_;
	}

	void Store(void* v) {
		ptr_ = v;
	};
};

#endif //!_MY_DB_MY_LOCK_H_