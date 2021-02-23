#include "memory.h"
#include <assert.h>

Memory::Memory() {
	alloctedSize_ = 0;
	unusePtr_ = NULL;
	unuseSize_ = 0;
}

Memory::~Memory() {
	for (size_t i = 0; i < blocks_.size(); i++) {
		delete[] blocks_[i];
	}
}

char* Memory::AllocateFallback(size_t bytes) {
	if (bytes >(blockSize >> 2)) {
		// 如果新需要的空间比较大，就直接新申请一个与之大小相符合的块，减少浪费剩余空间
		// 至于选1/8，因为首先得是偶数，大了浪费剩余空间会较多，小了没必要
		char* result = AllocateNewBlock(bytes);
		return result;
	}
	else {
		// 否则分配新的固定块
		this->unusePtr_ = AllocateNewBlock(blockSize);

		char* result = this->unusePtr_;
		this->unusePtr_ += bytes;
		this->unuseSize_ = blockSize - bytes;
		return result;
	}
}

char* Memory::AllocateNewBlock(size_t bytes) {
	char* result = new char[bytes];
	this->alloctedSize_ += bytes;
	blocks_.push_back(result);
	return result;
}

char* Memory::Allocate(size_t bytes) {
	char* result;
	alloclock_.lock();
	if (bytes <= this->unuseSize_) {
		result = this->unusePtr_;
		this->unusePtr_ += bytes;
		this->unuseSize_ -= bytes;
	}
	else result = AllocateFallback(bytes);
	alloclock_.unlock();
	return result;
}

char* Memory::AllocateAligned(size_t bytes) {
	const int align = sizeof(void*);    // 以此大小对齐

	alloclock_.lock();
	size_t mod = (size_t)(unusePtr_) & (align - 1);
	if (mod != 0) mod = align - mod;    // 现在mod代表为了对齐需要填充的字节数
	size_t needBytes = bytes + mod;
	char* result;

	if (needBytes <= this->unuseSize_) {
		result = this->unusePtr_ + mod;
		this->unusePtr_ += needBytes;
		this->unuseSize_ -= needBytes;
	}
	else {
		// 此函数总是返回对齐的内存
		result = AllocateFallback(bytes);
	}
	assert(((size_t)(result) & (align - 1)) == 0);  // 确认是否对齐
	alloclock_.unlock();

	return result;
}

size_t Memory::MemoryUsage() {
	size_t result = blocks_.capacity() * sizeof(char*);
	alloclock_.lock();
	result = result + this->alloctedSize_ - this->unuseSize_;
	alloclock_.unlock();
	return result;
}
