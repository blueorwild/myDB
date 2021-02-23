#include "skiplist.h"
#include "code.h"
#include "seq_num.h"
#include <iostream>

SkipList::SkipList(Memory* memory)
{
	this->maxLevel_ = 0;    // 初始化默认实际最大高度为1层，即下标为0
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
	// 比较Key
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
	Node* pNode = this->head_;   //从头开始找
	Node* next = NULL;

	for (int i = this->maxLevel_; i >= 0; i--) { // 必须从高层往底层
		while (true) {
			next = pNode->AtomicNext(i);
			if (next && InternalKeyIsLess(next->data, key, s)) {  // 这里比较的是internalKey
				pNode = next;
			}
			else break;
		}

	} // 循环终止，next指向大于等于key最小的节点

	if (next != NULL && KeyCompare(next->data, key) == 0) {
		if (DecodeForType(next->data) == 1) return 1;
		return -1;
	}
	return 0;
}

int SkipList::Read(const char* internalKey, std::string* value)
{
	Node* pNode = this->head_;   //从头开始找
	Node* next = NULL;

	// 保存一下用于后续比较，这样只需这里解码一次
	char key[256] = { 0 };
	DecodeForKey(internalKey, key);
	uint64_t s = DecodeForNum(internalKey);

	for (int i = this->maxLevel_; i >= 0; i--) { // 必须从高层往底层
		while (true) {
			next = pNode->AtomicNext(i);
			if (next && InternalKeyIsLess(next->data, key, s)) {  // 这里比较的是internalKey
				pNode = next;
			}
			else break;
		}

	} // 循环终止，next指向大于等于key最小的节点

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
	Node* pNode = this->head_;      // 从头开始找位置
	Node* next = NULL;
	Node* prevNodes[MAX_LEVEL];     // 存放需要更新的前置节点
	Node* nextNodes[MAX_LEVEL];     // 存放需要更新的前置节点的后置节点，用于提交时验证

	// 保存一下key,用于后续比较，这样只需这里解码一次
	char key[256] = { 0 };     
	DecodeForKey(data, key);

	int maxlevel = this->maxLevel_; // 统一用这个层高，以防中途改变引起的错误
	// 统计需要更新的前置节点
	for (int i = maxlevel; i >= 0; i--) {  // 必须从高层往底层
		while (true) {
		    next = pNode->AtomicNext(i);  // 先拿到next指针保证后面存的是没有变的
			if (next && KeyCompare(next->data, key)< 0 ) {  // 这里比较的是Key
				pNode = next;
			}
			else {
				prevNodes[i] = pNode;
				nextNodes[i] = next;
				break;
			}
		}
	}// 循环终止，next指向大于等于key最小的节点

	// 不能重复插入（即key值和value均相同）
	if (next != NULL && KeyIsEqual(next->data, data) &&
		ValueIsEqual(next->data, data)) return -1;

	// 生成随机层数的新节点
	int level = random_->RandomHeight(MAX_LEVEL);
	Node* newNode = NewNode(level, data);

	// 如果新生成的节点高度超出原有实际高度，高出的部分前置节点为head_
	if (level > maxlevel) {
		for (int i = maxlevel + 1; i <= level; i++) {
			prevNodes[i] = head_;
			nextNodes[i] = NULL;
		}
		maxlevel = level;
		// if (level > this->maxLevel_) this->maxLevel_ = level;
	}

	// 插入节点
	mutex_.lock();
	// 验证阶段
	// 如下情况会有冲突而导致写失败：
	// 1.前置节点的后置节点发生变化，且新的后置节点在要插入的值的前面
	for (int i = 0; i <= maxlevel; ++i) { // 从上至下，或者从下至上随意
		Node* pNext = prevNodes[i]->Next(i);
		if (pNext != nextNodes[i]) {
			if (KeyCompare(pNext->data, key) < 0) {
				mutex_.unlock();
				return 0;
			}
		}
	}

	if (maxlevel > this->maxLevel_) this->maxLevel_ = level;  // 记得原子

	// 新想法尝试，在写入之前才添加进序列号，这样做有如下益处
	// 1.节省序列号，保证每一个分出去的写序列号都是会发生的操作
	// 2.在写写互不阻塞的情况下，保证写序列号更大的操作“真正的”后完成(内存)
	// 如果在最开始分配序列号，由于并发可能会导致写序列号大的操作先完成，
	// 这个保证对快照读是必不可少的
	// 3.已经处在锁中，序列号的增加就不用考虑同步的问题了(其实不算益处)
	SequenceNumber* pSN = SequenceNumber::Initial();
	uint64_t writeSN = pSN->GetWriteSN();
	EncodeForSN(data, writeSN);

	// 更新阶段
	for (int i = 0; i <= level; i++) { // 必须从底层往高层
		// 此处两行代码分三步，
		// 1.读取前置节点的后置节点；2.设置新节点的后置节点为1中读取的；3.设置前置节点的后置节点为新节点。
		// 其中1和2不需要同步，因为1读的时候不存在其它的写操作，2写的时候不存在读到新节点的操作
		newNode->SetNext(i, prevNodes[i]->Next(i));
		prevNodes[i]->AtomicSetNext(i, newNode);
	}

	// 写完之后趁着还在锁里，直接把读序列号更新了
	// 这样可以保证读序列号肯定是递增的
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
	// 此函数具体方法是遍历，直到遇到第一个序列号<=s的条目，序列号>s的条目不属于清理范围。
	// 若该条目类型为删除，则成功找到返回
	// 若类型为新增，因为该条目是某段相同key值条目第一个序列号<=s的，不应清理，继续往后
	// 若下一条目为NULL,说明没有需要清理的条目，返回
	// 若下一条目key值与当前key值相同，则需要被清理，成功找到返回；
	// 若下一条目是不同key值，说明当前key值没有需要清理的条目，开始重复上述过程。
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
	// 此函数具体方法是遍历，传进来的指针指向的是要被清理的条目
	// 若下一条目key值与被清理条目相同，则也需被清理，继续往后
	// 若key不相同，且序列号大于s，则该条目不需被清理，成功找到返回
	// 若序列号<=s，且类型为删除，则也需被清理，继续往后
	// 若类型为新增，因为是某段相同key值条目第一个序列号<=s的，不应清理，成功找到返回

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
	// 此函数执行时没有其它写操作，比较安全(可能同时有读的操作，注意)
	// 具体方法是对所有层的节点（12层），逐层遍历，对找到的第一个需要清理的节点的key值进行记录
	// 并保存它的前置节点，然后找到下一个不需要清理的节点，直接把它设为保存的前置节点的后置节点
	// 重复此过程直至无可清理节点

	for (int i = MAX_LEVEL - 1; i >= 0; --i) { // 必须从顶层开始删除
		Node* prev = this->head_;
		while (true) {
			if (FindNeedClear(prev, s, i)) {
				Node* p = FindNotNeedClear(prev->Next(i), s, i);
				// 无论p是否为NULL，下行代码都没问题
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
	//根据当前节点的层高来分配不同大小的内存
	//内存大小为:结构本身 +　层数　* 每一层的占用的空间
	char* mem = memory_->AllocateAligned((sizeof(Node) + level * sizeof(AtomicPointer)));
	Node* pNode = new (mem) Node;   // placement new

	pNode->data = data;

	for (int i = 0; i <= level; i++) {
		pNode->SetNext(i, NULL);
	}
	return pNode;
}
