// �ڴ����������һ������ÿ���ڵ���һ��һ�η���Ĵ��ڴ棬����Ҫ�����ڴ�ʱ
// �ȴ���Щ�ѷ���õĴ��ڴ���г���ȡ���ڴ棬���ʧ�ܵĻ��ٳ�������ķ�����ڴ�
// ��Ч�ı����˴����ڴ���Ƭ�ĳ����Լ�Ƶ��С�ڴ�����˷�ʱ�䣬������ڴ����Ч��
// ���⿼�ǵ�AtomicPointer���Ҫ�󣬴˷�װҲʵ�����ڴ����ķ��䡣

#ifndef _MY_DB_MEMORY_H_
#define _MY_DB_MEMORY_H_

#include <vector>
#include "my_lock.h"

class Memory {
private:
	std::vector<char*> blocks_;     // �������ָ�룬ÿ��ָ��ָ��һ���ڴ�
	char* unusePtr_;                // ָ��ǰ�ڴ��δʹ�ÿռ����ַ
	size_t unuseSize_;              // ��ǰ�ڴ��δʹ�ÿռ�Ĵ�С
	size_t alloctedSize_;           // ��¼��ĿǰΪֹ�ѷ�����ڴ��ܴ�С

	static const int blockSize = 4096;       // 4KB,һ���ȽϺ��ʵ��ڴ���С
	SpinLock alloclock_;            // �����ڴ�����������

	// ��ǰ�ڴ��ʣ��ռ䲻��Ĵ�����
	char* AllocateFallback(size_t bytes);
	// �����µ��ڴ��
	char* AllocateNewBlock(size_t bytes);

public:
	Memory();
	~Memory();

	// ��ͨ���䣺����һ��ָ��bytes��С���ڴ���ַ��ַ��ָ��
	char* Allocate(size_t bytes);

	// ��֤�ڴ����ķ���(�׵�ַ����)
	char* AllocateAligned(size_t bytes);

	// ���ط�������ڴ桾��ʹ�á���(�������ѷ��䵫��δʹ�õĿռ�)��
	size_t MemoryUsage();

};

#endif  // !_MY_DB_MEMORY_H_
