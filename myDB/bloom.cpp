
#define SETBIT(bitset, i) (bitset[i >> 3] |= (1 << (i & 7)))  // ��������ĳ������λ��λ
#define GETBIT(bitset, i) (bitset[i >> 3]  & (1 << (i & 7)))  // ��ȡ������ĳ������λֵ

#include <memory.h>

#include "bloom.h"
#include "hashes.h"

Bloom::Bloom()
{
	this->bitset_ = (unsigned char*)calloc(this->size_>>3, sizeof(char));
}

Bloom::~Bloom()
{
	free(this->bitset_);
	this->bitset_ = NULL;
}

void Bloom::AddKey(const char* key)
{
	int num = 0;
	num = SaxHash(key) & (this->size_ - 1);
	SETBIT(this->bitset_, num);
	num = ElfHash(key) & (this->size_ - 1);
	SETBIT(this->bitset_, num);
	num = SdbmHash(key) & (this->size_ - 1);
	SETBIT(this->bitset_, num);
	num = MurmurHash(key) & (this->size_ - 1);
	SETBIT(this->bitset_, num);
	num = JenkinsHash(key) & (this->size_ - 1);
	SETBIT(this->bitset_, num);
}

bool Bloom::QueryKey(const char* key)
{
	int num = 0;
	num = SaxHash(key) & (this->size_ - 1);
	if (!GETBIT(this->bitset_, num)) return false;
	num = ElfHash(key) & (this->size_ - 1);
	if (!GETBIT(this->bitset_, num)) return false;
	num = SdbmHash(key) & (this->size_ - 1);
	if (!GETBIT(this->bitset_, num)) return false;
	num = MurmurHash(key) & (this->size_ - 1);
	if (!GETBIT(this->bitset_, num)) return false;
	num = JenkinsHash(key) & (this->size_ - 1);
	if (!GETBIT(this->bitset_, num)) return false;
	return true;
}
