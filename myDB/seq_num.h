// 全局通用的序列号，采用单例模式实现
// 分为读序列号和写序列号，写操作在开始时会获取写序列号并+1，成功写入后
// 才更新读序列号，如此可保证读到对应序列号最新的数据

#ifndef _MY_DB_SEQ_NUM_H_
#define _MY_DB_SEQ_NUM_H
#include <atomic>

class SequenceNumber {

private:
	uint64_t write_SN;
	std::atomic_uint64_t read_SN;
	SequenceNumber();
	static SequenceNumber *p;

public:
	static SequenceNumber *Initial() { 
		return p; 
	}

	// 获取当前write_SN,并执行+1操作,需要外部同步
	uint64_t GetWriteSN();

	// 获取当前read_SN，不需要同步(原子变量)
	uint64_t GetReadSN();

	// 更新当前read_SN,不需要外部同步
	void SetReadSN(uint64_t s);
};

#endif // ! _MY_DB_SEQ_NUM_H_
