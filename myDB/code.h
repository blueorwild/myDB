// ����kv��¼�ַ���������һЩ�����������������

#ifndef _MY_DB_CODE_H_
#define _MY_DB_CODE_H_

#include <string>

// ��data�н�����key,����key��
extern inline void DecodeForKey(const char* data, char* key);

// ��data�н��������к�
extern uint64_t DecodeForNum(const char* data);

// ��data�н������������ͣ�����1/ɾ��0��
extern int DecodeForType(const char* data);

// ��data�н�����value
extern void DecodeForValue(const char* data, std::string& value);

// ��ȡ�����ı����ĳ��ȣ�ÿ�ֽڵ�7λ��Ч��
extern size_t VarintLength(size_t varint);

// �����кŽ��б�����봫���ָ��
extern void EncodeForSN(char* p, uint64_t sequenceNumber);

// �����ݴ�С���б�����봫���ָ��
extern void EncodeForValueSize(char* &p, size_t valueSize);

#endif  // !_MY_DB_CODE_H_