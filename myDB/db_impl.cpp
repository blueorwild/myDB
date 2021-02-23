#include <assert.h>

#include "db_impl.h"
#include "code.h"

DBImpl::DBImpl()
{
	Memory* m = new Memory;
	this->memory_.Store(m);
	SkipList* p = new SkipList(m);
	this->head_.Store(p);
	this->SN_ = SequenceNumber::Initial();
	this->compact_ = new Compact;
	this->semaphore_ = new std::atomic_int(0);
}

DBImpl::~DBImpl()
{
}

SnapShot::~SnapShot() 
{
}

void DB::CreateDB(DB** pDB)
{
	(*pDB) = new DBImpl();
}

void DBImpl::DeleteDB()
{
	delete (Memory *)this->memory_.Load();
	delete (SkipList *)this->head_.Load();
	delete this->compact_;
	delete this->semaphore_;
}

void DBImpl::NeedCompact(int size) {
	Memory* m = (Memory*)this->memory_.Load();
	if (size + m->MemoryUsage() > this->maxSize_) {
		// 老表退休，新表上阵
		SkipList* p = (SkipList*)this->head_.Load();
		this->compact_->DoCompact(m, p, this->semaphore_);

		m = new Memory;
		p = new SkipList(m);
		this->head_.AtomicStore(p); // 有可能同时有读这个指针的情况发生
		this->memory_.AtomicStore(m);
		this->semaphore_ = new std::atomic_int(0);
	}
}

void DBImpl::Add(const char*key, const char* value)
{
	// 此函数功能是拼接好需要插入的字符串，具体插入工作交由SkipList去做
	
	// 计算插入数据串的总长度(其中‘1’用于存放keySize，‘8’用于存放序列号和类型)
	// 序列号64位8字节，因为用不了那么多32位可能又不够，为了节省空间左移1字节留给类型
	size_t keySize = strlen(key);
	size_t valueSize = strlen(value);
	size_t dataSize = 1 + keySize + 8 + VarintLength(valueSize) + valueSize;

	// 是否需要数据清理
	compactlock_.lock();

	NeedCompact(dataSize);
	SkipList* skipList = (SkipList*)this->head_.Load();
	Memory* memory = (Memory*)this->memory_.Load();
	std::atomic_int* semaphore = this->semaphore_;
	++*semaphore;   // 放在这里才安全

	compactlock_.unlock();

	// 一项项的拼接这个串（序列号留着最后写入）
	char* buf = memory->Allocate(dataSize);    // 这里不需要刻意对齐
	char* p = buf;
	
	*p++ = (char)keySize;
	memcpy(p, key, keySize);
	p += keySize;
	*(p + 7) = (char)1;
	p += 8;
	EncodeForValueSize(p, valueSize);
	memcpy(p, value, valueSize);

	// 插入
	int result = skipList->Write(buf);
	while (result == 0) {
		// 若有并发冲突，重新尝试写一次
		result = skipList->Write(buf);
	}
	--*semaphore;
}

void DBImpl::Delete(const char* key)
{
	// 类似上述Add，拼接好后交由SkipList去弄

	// 计算插入数据串的总长度
	size_t keySize = strlen(key);
	size_t dataSize = 1 + keySize + 8;

	// 是否需要数据清理
	compactlock_.lock();

	NeedCompact(dataSize);
	SkipList* skipList = (SkipList*)this->head_.Load();
	Memory* memory = (Memory*)this->memory_.Load();
	std::atomic_int* semaphore = this->semaphore_;
	++*semaphore;   // 放在这里才安全

	compactlock_.unlock();

	// 拼好(删除类型号为0，这里就不用单独拼一次了，后面序列号左移的时候自然会留下0)
	char* buf = memory->Allocate(dataSize);
	buf[0] = (char)keySize;
	memcpy(buf + 1, key, keySize);

	// 插入
	int result = skipList->Write(buf);
	while (result == 0) {
		// 若有并发冲突，重新尝试写一次
		result = skipList->Write(buf);
	}
	--*semaphore;
}

void DBImpl::Query(const SnapShot* snapShot, const char* key, std::string* value)
{
	// 类似的，拼接好后交由SkipList去弄
	// 获取查询序列号
	uint64_t sequenceNumber = 0;
	SnapShotImpl* s = (SnapShotImpl*)snapShot;
	if (!s) {
		sequenceNumber = this->SN_->GetReadSN();
	}
	else sequenceNumber = s->number_;

	// 拼一拼
	char internalKey[270] = { 0 };  // > (1 + 255 + 8)
	char* p = internalKey;
	int keySize = strlen(key);
	*p++ = keySize;
	memcpy(p, key, keySize);
	p += keySize;
	EncodeForSN(p, (sequenceNumber << 8) | 1);

	SkipList* skipList = (SkipList*)this->head_.AtomicLoad();
	if (skipList->Read(internalKey, value) == 0) {
		// 如果没有在当前活跃表里找到，转到已经清理过的去找
		// 但是注意可能当前活跃表在找的时候已经转到清理了，为了避免重复找，传过去指针
		this->compact_->Query(internalKey, value, skipList);
	}
}

void DBImpl::Alter(const char* key, const char* newValue)
{
	// 这里考虑了一下，就不先拼接了，效率可以更高
	// 此函数功能是先判断key值记录是否存在，若存在再写
	// 所以直接把key和读序列号传给SkipList判断是否存在，存在直接调用Add接口新增记录即可
	// 这样可能封装性不是很好，因为按理SkipList应该只收到一条完整的internalKey
	uint64_t readSN = this->SN_->GetReadSN();

	SkipList* skipList = (SkipList*)this->head_.AtomicLoad();
	int result = skipList->Contain(key, readSN);
	if (result == -1) return;
	if (result == 0) {
		// 类似Query，避免重复找
		if (!this->compact_->Contain(key, readSN, skipList))
			return;
	}	

	// 到这说明存在key的记录，可以修改
	Add(key, newValue);
}

const SnapShot* DBImpl::GetSnapShot()
{
	uint64_t s = this->SN_->GetReadSN();
	SnapShotList* p = SnapShotList::Initial();
	return p->New(s);
}

void DBImpl::DeleteSnapShot(const SnapShot* snapshot)
{
	SnapShotList* p = SnapShotList::Initial();
	p->Delete((const SnapShotImpl*)snapshot);
}
