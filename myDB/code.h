// 关于kv记录字符串操作的一些函数，包括编码解码

#ifndef _MY_DB_CODE_H_
#define _MY_DB_CODE_H_

#include <string>

// 从data中解码获得key,放入key中
extern inline void DecodeForKey(const char* data, char* key);

// 从data中解码获得序列号
extern uint64_t DecodeForNum(const char* data);

// 从data中解码获得数据类型（新增1/删除0）
extern int DecodeForType(const char* data);

// 从data中解码获得value
extern void DecodeForValue(const char* data, std::string& value);

// 获取变量的编码后的长度（每字节低7位有效）
extern size_t VarintLength(size_t varint);

// 把序列号进行编码放入传入的指针
extern void EncodeForSN(char* p, uint64_t sequenceNumber);

// 把数据大小进行编码放入传入的指针
extern void EncodeForValueSize(char* &p, size_t valueSize);

#endif  // !_MY_DB_CODE_H_