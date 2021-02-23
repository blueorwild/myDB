#include <thread>

#include "compact.h"
#include "seq_num.h"
#include "code.h"

Compact::Compact()
{
	this->head_ = new Node;
	this->head_->next_ = NULL;
}

Compact::~Compact()
{
	Node* p = this->head_->next_;
	while (p) {
		Node* pNext = p->next_;
		delete p->memory_;
		delete p->skiplist_;
		delete p->bloom_;
		p = pNext;
	}

	delete this->head_;
}

void Compact::DoCompact(Memory* memory, SkipList* skiplist, std::atomic_int* semaphore)
{
    // �Ѵ������ı�������������ķŵ����棬�������ñ������������ȽϿ�
	// ������Ҫ��������ʱҲ���������µı��в���,����һ���ҵ��Ͳ�����������
	Node* pNode = new Node;
	pNode->memory_ = memory;
	pNode->skiplist_ = skiplist;
	// ������������Ҫʱ�䣬�п�����ʱ�Ѿ��в�ѯ�����ˣ��������ɺ������滻����
	pNode->bloom_ = NULL;  

	// ����ѹ��һ����϶���д��һ�ű�죬���ﲻ������������
	pNode->next_ = this->head_->next_;
	this->head_->next_ = pNode;         

	// Ȼ���skiplist����������͹��������ɿ���̨�߳��ܣ�������������д
	// ������һ�δ���Ҳ�Ž��̵߳�ԭ����Ҫ��֤�Ѵ���ѹ������skiplist������˲��������µ�skiplist
	std::thread t(&Compact::CompactPrep, this, skiplist, semaphore, pNode);
    t.detach();
}

void Compact::CompactPrep(SkipList* skiplist, std::atomic_int* semaphore, Node* pNode)
{
	// ���Ȼ�ȡ������������кţ�����С��snapshot�ţ���û��snapshot������ݵ�ǰ����readSN��
	uint64_t s = 0;
	SnapShotList* pSnapShot = SnapShotList::Initial();
	if (!pSnapShot->empty()) {
		s = pSnapShot->oldest()->number_;
	}
	else {
		SequenceNumber* pSN = SequenceNumber::Initial();
		s = pSN->GetReadSN();
	}

	// Ȼ��ȴ����ڴ˱��ϵ�д�������
	while (*semaphore != 0) Sleep(10);  // û�������ȴ�һ��ʱ��
	delete semaphore; // ���ź���ʹ�������

	// �������������ɽ�������
	skiplist->DataMerge(s);

	// ������ɺ����ɹ�����
	Bloom* bloom = new Bloom();
	skiplist->AddBloomFilter(bloom);
	pNode->bloom_ = bloom;
}

void Compact::Query(const char* internalKey, std::string* value, SkipList* skipList)
{
	// ������������б������ң��ҵ���ȷ�ļ����أ�������ȷ����ָ�����µġ�
	Node* pNode = this->head_->next_;
	if (pNode == NULL) return;

	// �п��ܲ���ı�ո�ת���������У���������
	// ���ǲ���һ�ε�ʱ��д��һ���ű��������ܣ����������һ��
	if (pNode->skiplist_ == skipList) pNode = pNode->next_;

	char key[256] = { 0 };
	DecodeForKey(internalKey, key);
	while (pNode) {
		// ����������Ѿ����ɺ��ˣ���ȥ����������
		if (pNode->bloom_ && !pNode->bloom_->QueryKey(key)) { 
			pNode = pNode->next_;
		}
		else{
			if (pNode->skiplist_->Read(internalKey, value) == 0) pNode = pNode->next_;
			else return;
		}
	}
}

bool Compact::Contain(const char* key, uint64_t s, SkipList* skipList)
{
	// ������������б������ң��ҵ��򷵻�true,
	Node* pNode = this->head_->next_;
	if (pNode == NULL) return false;
	// ���������Query
	if (pNode->skiplist_ == skipList) pNode = pNode->next_;
	while (!pNode) {
		if (pNode->bloom_ && !pNode->bloom_->QueryKey(key)) {
			pNode = pNode->next_;
		}
		else {
			int result = pNode->skiplist_->Contain(key, s);
			if (result == 0) pNode = pNode->next_;
			else if (result == 1) return true;
			else return false;
		}
	}
	return false;
}
