#include "code.h"

inline void DecodeForKey(const char* data, char* key)
{
	size_t keySize = (unsigned char)data[0];
	memcpy(key, data + 1, keySize);
	/*
	for (size_t i = 0; i < keySize; i++) {
		key[i] = *++data;
	}
	*/
}

uint64_t DecodeForNum(const char* data)
{
	size_t keySize = (unsigned char)data[0];
	++data;
	data += keySize;
	uint64_t sequenceNumber = 0;
	for (int i = 0; i < 7; i++) {
		sequenceNumber <<= 8;
		sequenceNumber |= (unsigned char)*data++;
	}
	return sequenceNumber;
}

int DecodeForType(const char* data)
{
	size_t keySize = (unsigned char)data[0];
	data += 8 + keySize;
	return *data;
}

void DecodeForValue(const char* data, std::string& value)
{
	size_t keySize = (unsigned char)data[0];
	++data;
	data += keySize;
	data += 8;

	// 获取valueSize并跳过存储valueSize的地址
	size_t valueSize = 0;
	for (int i = 0; ; ++i, ++data) {
		valueSize |= (unsigned char)(*data & 127) << (7 * i);
		if ((*data & 128) == 0) break;
	}
	++data;
	// 为了让string类型的value知道自己的长度,等价于value = std::string(data, valueSize);
	std::string str(data, valueSize);
	value = str;
}

size_t VarintLength(size_t varint)
{
	int len = 1;
	size_t num = 128;
	while (varint >= num) {
		varint >>= 7;
		len++;
	}
	return len;
}

void EncodeForSN(char* p, uint64_t sequenceNumber)
{
	// sequenceNumber是按照大端方式存的，即低位地址存放高字节
	// 这样读写都比较方便
	//sequenceNumber <<= 8;
	size_t keySize = (unsigned char)(*p);
	++p;
	p += keySize;
	for (int i = 0; i < 7; ++i) {
		sequenceNumber <<= 8;
		*p++ = (unsigned char)(sequenceNumber >> 56);
	}
}

void EncodeForValueSize(char* &p, size_t valueSize)   // 可优化
{
	// valueSize的存储每字节只低7位有效，最高位1代表未结束，0代表结束
	// 按照小端的方式存，低地址存低位字节，因为一开始计算最高位比较麻烦
	while (valueSize >= 128) { // 1000 0000
		*p++ = (char)(valueSize | 128);
		valueSize >>= 7;
	}
	*p++ = (char)valueSize;
}
