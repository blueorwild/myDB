// 此处包括所有有关锁的操作

#ifndef _MY_DB_MY_LOCK_H_
#define _MY_DB_MY_LOCK_H_

#include <windows.h>
#include <atomic>
#include <mutex>

// 自旋锁
class SpinLock{
private:
	long int flag_;

public:
	SpinLock() { 
		// flag_置为‘0’
		flag_ = 0;
	}

	void lock(){
		while (InterlockedExchange(&flag_, 1)) Sleep(0);
	}

	void unlock(){
		InterlockedExchange(&flag_, 0);
	}
};

// 自旋锁2
class SpinLock2 {
private:
	std::atomic_flag flag_;

public:
	SpinLock2() {
		// flag_置为‘0’
		flag_.clear();
	}

	void lock() {
		while (flag_.test_and_set()) Sleep(0);
	}

	void unlock() {
		flag_.clear();
	}
};

// 互斥锁（直接用库文件的，没什么好说的）
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

// 无锁原子操作(原子指针)
class AtomicPointer {
private:
	void * ptr_;
public:
	AtomicPointer() : ptr_(NULL) {}

	// Interlocked函数的工作原理取决于代码运行的CPU平台，如果是x86系列CPU，
	// 那么Interlocked函数会在总线上维持一个硬件信号，这个信号会阻止其他CPU
	// 访问同一个内存地址。另外必须确保传给这些函数的变量地址是经过对齐的

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