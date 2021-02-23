// 一个非常简单的布隆过滤器，大小就设为定值了

#ifndef _MY_DB_BLOOM_H_
#define _MY_DB_BLOOM_H_

#include <stdlib.h>

class Bloom {
private:
	unsigned char* bitset_;            // 过滤器的二进制串结构 

	// 过滤器的大小（bit位数，为了速度强制成2的幂次）
	// 根据一个skipList大小4MB，按平均每条kv记录64B计，每个skiplist有64k条记录
	// 按照平均每个key有2条记录，每个skiplist有32k条不同key
	// 哈希函数有4个，对给定的n,k,把m设成16n，假正率约为千分之二，可以接受？
	static const int size_ = 1<<20;    //16*32*1024  

public:
	Bloom();
	~Bloom();

	// 往过滤器添加一个key
	void AddKey(const char* key);

	// 在过滤器查找一个key
	bool QueryKey(const char* key);

};

#endif // !_MY_DB_BLOOM_H_