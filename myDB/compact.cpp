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
    // 把传进来的表放入链，先来的放到后面，这样不用遍历整个链，比较快
	// 另外需要查找数据时也是先在最新的表中查找,这样一旦找到就不用往后找了
	Node* pNode = new Node;
	pNode->memory_ = memory;
	pNode->skiplist_ = skiplist;
	// 过滤器生成需要时间，有可能这时已经有查询操作了，后面生成好了再替换进来
	pNode->bloom_ = NULL;  

	// 考虑压缩一个表肯定比写满一张表快，这里不发生并发问题
	pNode->next_ = this->head_->next_;
	this->head_->next_ = pNode;         

	// 然后对skiplist的数据清理和过滤器生成开后台线程跑，不阻塞正常读写
	// 不把上一段代码也放进线程的原因是要保证把传给压缩器的skiplist保存好了才能生成新的skiplist
	std::thread t(&Compact::CompactPrep, this, skiplist, semaphore, pNode);
    t.detach();
}

void Compact::CompactPrep(SkipList* skiplist, std::atomic_int* semaphore, Node* pNode)
{
	// 首先获取清理所需的序列号，即最小的snapshot号，若没有snapshot，则根据当前最新readSN号
	uint64_t s = 0;
	SnapShotList* pSnapShot = SnapShotList::Initial();
	if (!pSnapShot->empty()) {
		s = pSnapShot->oldest()->number_;
	}
	else {
		SequenceNumber* pSN = SequenceNumber::Initial();
		s = pSN->GetReadSN();
	}

	// 然后等待正在此表上的写操作完成
	while (*semaphore != 0) Sleep(10);  // 没有完成则等待一段时间
	delete semaphore; // 此信号量使命已完成

	// 具体清理工作交由交由跳表
	skiplist->DataMerge(s);

	// 清理完成后生成过滤器
	Bloom* bloom = new Bloom();
	skiplist->AddBloomFilter(bloom);
	pNode->bloom_ = bloom;
}

void Compact::Query(const char* internalKey, std::string* value, SkipList* skipList)
{
	// 在清理过的所有表里面找，找到正确的即返回，这里正确的是指“最新的”
	Node* pNode = this->head_->next_;
	if (pNode == NULL) return;

	// 有可能查过的表刚刚转过来清理中，跳过它，
	// 考虑查找一次的时间写满一整张表几乎不可能，所以最多跳一次
	if (pNode->skiplist_ == skipList) pNode = pNode->next_;

	char key[256] = { 0 };
	DecodeForKey(internalKey, key);
	while (pNode) {
		// 如果过滤器已经生成好了，先去过滤器看看
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
	// 在清理过的所有表里面找，找到则返回true,
	Node* pNode = this->head_->next_;
	if (pNode == NULL) return false;
	// 类似上面的Query
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
