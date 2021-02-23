#include "skiplist.h"
#include "code.h"
#include "seq_num.h"
#include <iostream>

SkipList::SkipList(Memory* memory)
{
	this->maxLevel_ = 0;    // ��ʼ��Ĭ��ʵ�����߶�Ϊ1�㣬���±�Ϊ0
	this->memory_ = memory;
	this->head_ = NewNode(MAX_LEVEL - 1, NULL);
	this->random_ = new Random(16807);  // 7^5
}

SkipList::~SkipList()
{
	delete this->random_;
}

bool SkipList::InternalKeyIsLess(const char* data, const char* key, uint64_t s)
{
	char data_key[256] = { 0 };
	DecodeForKey(data, data_key);
	int result = strcmp(data_key, key);

	if (result < 0) return true;
	if (result == 0) {
		uint64_t a_s = DecodeForNum(data);
		if (a_s > s) return true;
	}
	return false;
}

int SkipList::KeyCompare(const char* data, const char* key)
{
	char data_key[256] = { 0 };
	DecodeForKey(data, data_key);
	return strcmp(data_key, key);
}

bool SkipList::KeyIsEqual(const char* a, const char* b)
{
	// �Ƚ�Key
	char a_key[256] = { 0 }, b_key[256] = { 0 };
	DecodeForKey(a, a_key);
	DecodeForKey(b, b_key);

	if (strcmp(a_key, b_key) == 0) return true;
	return false;
}

bool SkipList::ValueIsEqual(const char* a, const char* b)
{
	std::string a_value = "";
	std::string b_value = "";
	DecodeForValue(a, a_value);
	DecodeForValue(b, b_value);
	if (a_value == b_value) return true;
	return false;
}

int SkipList::Contain(const char* key, uint64_t s)
{
	Node* pNode = this->head_;   //��ͷ��ʼ��
	Node* next = NULL;

	for (int i = this->maxLevel_; i >= 0; i--) { // ����Ӹ߲����ײ�
		while (true) {
			next = pNode->AtomicNext(i);
			if (next && InternalKeyIsLess(next->data, key, s)) {  // ����Ƚϵ���internalKey
				pNode = next;
			}
			else break;
		}

	} // ѭ����ֹ��nextָ����ڵ���key��С�Ľڵ�

	if (next != NULL && KeyCompare(next->data, key) == 0) {
		if (DecodeForType(next->data) == 1) return 1;
		return -1;
	}
	return 0;
}

int SkipList::Read(const char* internalKey, std::string* value)
{
	Node* pNode = this->head_;   //��ͷ��ʼ��
	Node* next = NULL;

	// ����һ�����ں����Ƚϣ�����ֻ���������һ��
	char key[256] = { 0 };
	DecodeForKey(internalKey, key);
	uint64_t s = DecodeForNum(internalKey);

	for (int i = this->maxLevel_; i >= 0; i--) { // ����Ӹ߲����ײ�
		while (true) {
			next = pNode->AtomicNext(i);
			if (next && InternalKeyIsLess(next->data, key, s)) {  // ����Ƚϵ���internalKey
				pNode = next;
			}
			else break;
		}

	} // ѭ����ֹ��nextָ����ڵ���key��С�Ľڵ�

	if (next != NULL && KeyCompare(next->data, key) == 0){
		if (DecodeForType(next->data) == 1) {
			DecodeForValue(next->data, *value);
			return 1;
		}
		return -1;
	}
	return 0;
}

int SkipList::Write(char* data)
{
	Node* pNode = this->head_;      // ��ͷ��ʼ��λ��
	Node* next = NULL;
	Node* prevNodes[MAX_LEVEL];     // �����Ҫ���µ�ǰ�ýڵ�
	Node* nextNodes[MAX_LEVEL];     // �����Ҫ���µ�ǰ�ýڵ�ĺ��ýڵ㣬�����ύʱ��֤

	// ����һ��key,���ں����Ƚϣ�����ֻ���������һ��
	char key[256] = { 0 };     
	DecodeForKey(data, key);

	int maxlevel = this->maxLevel_; // ͳһ�������ߣ��Է���;�ı�����Ĵ���
	// ͳ����Ҫ���µ�ǰ�ýڵ�
	for (int i = maxlevel; i >= 0; i--) {  // ����Ӹ߲����ײ�
		while (true) {
		    next = pNode->AtomicNext(i);  // ���õ�nextָ�뱣֤��������û�б��
			if (next && KeyCompare(next->data, key)< 0 ) {  // ����Ƚϵ���Key
				pNode = next;
			}
			else {
				prevNodes[i] = pNode;
				nextNodes[i] = next;
				break;
			}
		}
	}// ѭ����ֹ��nextָ����ڵ���key��С�Ľڵ�

	// �����ظ����루��keyֵ��value����ͬ��
	if (next != NULL && KeyIsEqual(next->data, data) &&
		ValueIsEqual(next->data, data)) return -1;

	// ��������������½ڵ�
	int level = random_->RandomHeight(MAX_LEVEL);
	Node* newNode = NewNode(level, data);

	// ��������ɵĽڵ�߶ȳ���ԭ��ʵ�ʸ߶ȣ��߳��Ĳ���ǰ�ýڵ�Ϊhead_
	if (level > maxlevel) {
		for (int i = maxlevel + 1; i <= level; i++) {
			prevNodes[i] = head_;
			nextNodes[i] = NULL;
		}
		maxlevel = level;
		// if (level > this->maxLevel_) this->maxLevel_ = level;
	}

	// ����ڵ�
	mutex_.lock();
	// ��֤�׶�
	// ����������г�ͻ������дʧ�ܣ�
	// 1.ǰ�ýڵ�ĺ��ýڵ㷢���仯�����µĺ��ýڵ���Ҫ�����ֵ��ǰ��
	for (int i = 0; i <= maxlevel; ++i) { // �������£����ߴ�����������
		Node* pNext = prevNodes[i]->Next(i);
		if (pNext != nextNodes[i]) {
			if (KeyCompare(pNext->data, key) < 0) {
				mutex_.unlock();
				return 0;
			}
		}
	}

	if (maxlevel > this->maxLevel_) this->maxLevel_ = level;  // �ǵ�ԭ��

	// ���뷨���ԣ���д��֮ǰ����ӽ����кţ��������������洦
	// 1.��ʡ���кţ���֤ÿһ���ֳ�ȥ��д���кŶ��ǻᷢ���Ĳ���
	// 2.��дд��������������£���֤д���кŸ���Ĳ����������ġ������(�ڴ�)
	// ������ʼ�������кţ����ڲ������ܻᵼ��д���кŴ�Ĳ�������ɣ�
	// �����֤�Կ��ն��Ǳز����ٵ�
	// 3.�Ѿ��������У����кŵ����ӾͲ��ÿ���ͬ����������(��ʵ�����洦)
	SequenceNumber* pSN = SequenceNumber::Initial();
	uint64_t writeSN = pSN->GetWriteSN();
	EncodeForSN(data, writeSN);

	// ���½׶�
	for (int i = 0; i <= level; i++) { // ����ӵײ����߲�
		// �˴����д����������
		// 1.��ȡǰ�ýڵ�ĺ��ýڵ㣻2.�����½ڵ�ĺ��ýڵ�Ϊ1�ж�ȡ�ģ�3.����ǰ�ýڵ�ĺ��ýڵ�Ϊ�½ڵ㡣
		// ����1��2����Ҫͬ������Ϊ1����ʱ�򲻴���������д������2д��ʱ�򲻴��ڶ����½ڵ�Ĳ���
		newNode->SetNext(i, prevNodes[i]->Next(i));
		prevNodes[i]->AtomicSetNext(i, newNode);
	}

	// д��֮����Ż������ֱ�ӰѶ����кŸ�����
	// �������Ա�֤�����кſ϶��ǵ�����
	pSN->SetReadSN(writeSN);
	mutex_.unlock();
	return 1;
}

void SkipList::AddBloomFilter(Bloom* bloom)
{
	Node* p = this->head_->Next(0);
	while (p) {
		char key[256] = { 0 };
		DecodeForKey(p->data, key);
		bloom->AddKey(key);
		p = p->Next(0);
	}
}

bool SkipList::FindNeedClear(Node* &p, uint64_t s, int level)
{
	// �˺������巽���Ǳ�����ֱ��������һ�����к�<=s����Ŀ�����к�>s����Ŀ����������Χ��
	// ������Ŀ����Ϊɾ������ɹ��ҵ�����
	// ������Ϊ��������Ϊ����Ŀ��ĳ����ͬkeyֵ��Ŀ��һ�����к�<=s�ģ���Ӧ������������
	// ����һ��ĿΪNULL,˵��û����Ҫ�������Ŀ������
	// ����һ��Ŀkeyֵ�뵱ǰkeyֵ��ͬ������Ҫ�������ɹ��ҵ����أ�
	// ����һ��Ŀ�ǲ�ͬkeyֵ��˵����ǰkeyֵû����Ҫ�������Ŀ����ʼ�ظ��������̡�
	Node* next = p->Next(level);

	while (next != NULL) {
		if (DecodeForNum(next->data) <= s) {
			if (DecodeForType(next->data) == 0) 
				return true;
			else {
				p = next;
				next = p->Next(level);
				if (next == NULL) 
					return false;
				if (KeyIsEqual(p->data, next->data)) 
					return true;
			}
		}
		else {
			p = next;
			next = p->Next(level);
		}
	}
	return false;
}

SkipList::Node* SkipList::FindNotNeedClear(Node* p, uint64_t s, int level)
{
	// �˺������巽���Ǳ�������������ָ��ָ�����Ҫ���������Ŀ
	// ����һ��Ŀkeyֵ�뱻������Ŀ��ͬ����Ҳ�豻������������
	// ��key����ͬ�������кŴ���s�������Ŀ���豻�����ɹ��ҵ�����
	// �����к�<=s��������Ϊɾ������Ҳ�豻������������
	// ������Ϊ��������Ϊ��ĳ����ͬkeyֵ��Ŀ��һ�����к�<=s�ģ���Ӧ�����ɹ��ҵ�����

	Node* next = p->Next(level);

	while (next != NULL) {
		if (KeyIsEqual(p->data, next->data));
		else if (DecodeForNum(next->data) > s) 
			return next;
		else if (DecodeForType(next->data) == 1) 
			return next;

		p = next;
		next = p->Next(level);
	}
	return NULL;
}

void SkipList::DataMerge(uint64_t s)
{
	// �˺���ִ��ʱû������д�������Ƚϰ�ȫ(����ͬʱ�ж��Ĳ�����ע��)
	// ���巽���Ƕ����в�Ľڵ㣨12�㣩�������������ҵ��ĵ�һ����Ҫ����Ľڵ��keyֵ���м�¼
	// ����������ǰ�ýڵ㣬Ȼ���ҵ���һ������Ҫ����Ľڵ㣬ֱ�Ӱ�����Ϊ�����ǰ�ýڵ�ĺ��ýڵ�
	// �ظ��˹���ֱ���޿�����ڵ�

	for (int i = MAX_LEVEL - 1; i >= 0; --i) { // ����Ӷ��㿪ʼɾ��
		Node* prev = this->head_;
		while (true) {
			if (FindNeedClear(prev, s, i)) {
				Node* p = FindNotNeedClear(prev->Next(i), s, i);
				// ����p�Ƿ�ΪNULL�����д��붼û����
				prev->AtomicSetNext(i, p);
				if (p == NULL) break;
				prev = p;
			}
			else break;
		}
	}
}

SkipList::Node* SkipList::NewNode(int level, char* data)
{
	//���ݵ�ǰ�ڵ�Ĳ�������䲻ͬ��С���ڴ�
	//�ڴ��СΪ:�ṹ���� +��������* ÿһ���ռ�õĿռ�
	char* mem = memory_->AllocateAligned((sizeof(Node) + level * sizeof(AtomicPointer)));
	Node* pNode = new (mem) Node;   // placement new

	pNode->data = data;

	for (int i = 0; i <= level; i++) {
		pNode->SetNext(i, NULL);
	}
	return pNode;
}
