#ifndef _MY_DB_SKIPLIST_H_
#define _MY_DB_SKIPLIST_H_

#include <string>
#include <assert.h>

#include "memory.h"
#include "my_lock.h"
#include "random.h"
#include "bloom.h"

class SkipList {

private: 
	static const int MAX_LEVEL = 12;   // �����������߶ȣ�0~11��
	int maxLevel_;                     // ����ʵ�����߶�

	// ����ڵ�ṹ��
	struct Node {            
	public:
		char* data;     // �ڵ��ֵ(��������ƴ�ӳ�һ���ַ���)

		// ԭ���ԵĶ�
		Node* AtomicNext(int n) {
			return (Node*)(next_[n].AtomicLoad());
		}
		// ԭ���Ե�д
		void AtomicSetNext(int n, Node* x) {
			next_[n].AtomicStore(x);
		}
		// ��ͨ��
		Node* Next(int n) {
			return (Node*)(next_[n].Load());
		}
		// ��ͨд
		void SetNext(int n, Node* x) {
			this->next_[n].Store(x);
		}

	private:
		// ����ڵ��ָ������,ֻ�ܷ��ں��棬��Ϊ���ָ��������Ҫ��̬����
		// ������Ŀռ��������ģ��������data��ǰ�棬������ʱ��Ḳ�ǵ�data
		AtomicPointer next_[1];     
	
	};
	Node* head_;              // ����ͷ�ڵ�

	Memory *memory_;          // ͳһ���ڴ���䣬�����ϲ�Ĵ���
	Random *random_;          // ��������������������Ӧ�ļ�
	MutexLock  mutex_;         // ������(����Write����)

public:
	SkipList(Memory *memory);
	~SkipList();

	// *****************���¹��ϲ����********************
	// �ж�SkipList���Ƿ���ڶ�Ӧ���к�С��s��key�ļ�¼
	// ����ֵ -1 ����϶�������
	//        0 ����ǰ������
	//        1 �����Ѿ��ҵ�
	int Contain(const char* key, uint64_t s);

	// ����internalKey��SkipList�ж�ȡ��Ӧ��valueֵ���������ڷ���false
	// internalKey���û�key�����кźͼ�¼���ͣ�����/ɾ����ƴ�Ӷ���
	// ����ֵ -1 ����϶�������
	//        0 ����ǰ������
	//        1 �����Ѿ��ҵ�
	int Read(const char* internalKey, std::string* value);

	// ��SkipList�в�������
	// ����ֵ -1 ����ʧ�ܣ������ظ�����
	//        0 ����ʧ�ܣ������ᵼ�´���Ĳ�����ͻ
	//        1 ����ɹ�
	int Write(char* data);

	// �����������巽���Ƕ� ���к�<=s�ļ�¼ ֻ�������к������Ǹ���
	// ������к������Ǹ�����Ϊɾ������Ҳһ�������
	void DataMerge(uint64_t s);

	// �ѵ�ǰskiplist��key�Ž���������bloom��������
	void AddBloomFilter(Bloom* bloom);
	// *****************���Ϲ���������********************

	// *****************���¹�����ʹ��********************
	// a��bΪNode�е�data������ͨ�ã�
	// �ж�data��Key��Key(�û�key)�Ĵ�С��ϵ����С�����������Key�ֵ�����
	int KeyCompare(const char* data, const char* key);

	// �ж�a��Key��b��Key�Ƿ����(�û�key)
	bool KeyIsEqual(const char* a, const char* b);

	// �ж�data��InternalKey�Ƿ��С����С�����������key�ֵ�����, ���кŵĽ���
	bool InternalKeyIsLess(const char* data, const char* key, uint64_t s);

	// �ж�a��b��value�Ƿ���ͬ������ֱ����strcmp�Ƚϣ���Ϊû�н�������
	bool ValueIsEqual(const char* a, const char* b);

	// ��������������˳���ҵ�ĳ���е�һ����Ҫ����Ľڵ㣬
	// p���ڱ�����Ҫ����ڵ��ǰ�ýڵ㣬levelΪ����
	bool FindNeedClear(Node* &p, uint64_t s, int level);

	// ������������pΪ��Ҫ����Ľڵ㣬levelΪ����
	// �����ǰ�˳���ҵ�ĳ��p���һ������Ҫ����Ľڵ㣬
	Node* FindNotNeedClear(Node* p, uint64_t s, int level);

	// ����һ��ֵΪdata������Ϊlevel����SkipList�ڵ�
	Node* NewNode(int level, char* data);
};

#endif // !_MY_DB_SKIPLIST_H_