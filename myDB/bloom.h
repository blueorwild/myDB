// һ���ǳ��򵥵Ĳ�¡����������С����Ϊ��ֵ��

#ifndef _MY_DB_BLOOM_H_
#define _MY_DB_BLOOM_H_

#include <stdlib.h>

class Bloom {
private:
	unsigned char* bitset_;            // �������Ķ����ƴ��ṹ 

	// �������Ĵ�С��bitλ����Ϊ���ٶ�ǿ�Ƴ�2���ݴΣ�
	// ����һ��skipList��С4MB����ƽ��ÿ��kv��¼64B�ƣ�ÿ��skiplist��64k����¼
	// ����ƽ��ÿ��key��2����¼��ÿ��skiplist��32k����ͬkey
	// ��ϣ������4�����Ը�����n,k,��m���16n��������ԼΪǧ��֮�������Խ��ܣ�
	static const int size_ = 1<<20;    //16*32*1024  

public:
	Bloom();
	~Bloom();

	// �����������һ��key
	void AddKey(const char* key);

	// �ڹ���������һ��key
	bool QueryKey(const char* key);

};

#endif // !_MY_DB_BLOOM_H_