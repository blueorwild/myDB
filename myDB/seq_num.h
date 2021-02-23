// ȫ��ͨ�õ����кţ����õ���ģʽʵ��
// ��Ϊ�����кź�д���кţ�д�����ڿ�ʼʱ���ȡд���кŲ�+1���ɹ�д���
// �Ÿ��¶����кţ���˿ɱ�֤������Ӧ���к����µ�����

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

	// ��ȡ��ǰwrite_SN,��ִ��+1����,��Ҫ�ⲿͬ��
	uint64_t GetWriteSN();

	// ��ȡ��ǰread_SN������Ҫͬ��(ԭ�ӱ���)
	uint64_t GetReadSN();

	// ���µ�ǰread_SN,����Ҫ�ⲿͬ��
	void SetReadSN(uint64_t s);
};

#endif // ! _MY_DB_SEQ_NUM_H_
