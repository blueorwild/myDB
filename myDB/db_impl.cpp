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
		// �ϱ����ݣ��±�����
		SkipList* p = (SkipList*)this->head_.Load();
		this->compact_->DoCompact(m, p, this->semaphore_);

		m = new Memory;
		p = new SkipList(m);
		this->head_.AtomicStore(p); // �п���ͬʱ�ж����ָ����������
		this->memory_.AtomicStore(m);
		this->semaphore_ = new std::atomic_int(0);
	}
}

void DBImpl::Add(const char*key, const char* value)
{
	// �˺���������ƴ�Ӻ���Ҫ������ַ�����������빤������SkipListȥ��
	
	// ����������ݴ����ܳ���(���С�1�����ڴ��keySize����8�����ڴ�����кź�����)
	// ���к�64λ8�ֽڣ���Ϊ�ò�����ô��32λ�����ֲ�����Ϊ�˽�ʡ�ռ�����1�ֽ���������
	size_t keySize = strlen(key);
	size_t valueSize = strlen(value);
	size_t dataSize = 1 + keySize + 8 + VarintLength(valueSize) + valueSize;

	// �Ƿ���Ҫ��������
	compactlock_.lock();

	NeedCompact(dataSize);
	SkipList* skipList = (SkipList*)this->head_.Load();
	Memory* memory = (Memory*)this->memory_.Load();
	std::atomic_int* semaphore = this->semaphore_;
	++*semaphore;   // ��������Ű�ȫ

	compactlock_.unlock();

	// һ�����ƴ������������к��������д�룩
	char* buf = memory->Allocate(dataSize);    // ���ﲻ��Ҫ�������
	char* p = buf;
	
	*p++ = (char)keySize;
	memcpy(p, key, keySize);
	p += keySize;
	*(p + 7) = (char)1;
	p += 8;
	EncodeForValueSize(p, valueSize);
	memcpy(p, value, valueSize);

	// ����
	int result = skipList->Write(buf);
	while (result == 0) {
		// ���в�����ͻ�����³���дһ��
		result = skipList->Write(buf);
	}
	--*semaphore;
}

void DBImpl::Delete(const char* key)
{
	// ��������Add��ƴ�Ӻú���SkipListȥŪ

	// ����������ݴ����ܳ���
	size_t keySize = strlen(key);
	size_t dataSize = 1 + keySize + 8;

	// �Ƿ���Ҫ��������
	compactlock_.lock();

	NeedCompact(dataSize);
	SkipList* skipList = (SkipList*)this->head_.Load();
	Memory* memory = (Memory*)this->memory_.Load();
	std::atomic_int* semaphore = this->semaphore_;
	++*semaphore;   // ��������Ű�ȫ

	compactlock_.unlock();

	// ƴ��(ɾ�����ͺ�Ϊ0������Ͳ��õ���ƴһ���ˣ��������к����Ƶ�ʱ����Ȼ������0)
	char* buf = memory->Allocate(dataSize);
	buf[0] = (char)keySize;
	memcpy(buf + 1, key, keySize);

	// ����
	int result = skipList->Write(buf);
	while (result == 0) {
		// ���в�����ͻ�����³���дһ��
		result = skipList->Write(buf);
	}
	--*semaphore;
}

void DBImpl::Query(const SnapShot* snapShot, const char* key, std::string* value)
{
	// ���Ƶģ�ƴ�Ӻú���SkipListȥŪ
	// ��ȡ��ѯ���к�
	uint64_t sequenceNumber = 0;
	SnapShotImpl* s = (SnapShotImpl*)snapShot;
	if (!s) {
		sequenceNumber = this->SN_->GetReadSN();
	}
	else sequenceNumber = s->number_;

	// ƴһƴ
	char internalKey[270] = { 0 };  // > (1 + 255 + 8)
	char* p = internalKey;
	int keySize = strlen(key);
	*p++ = keySize;
	memcpy(p, key, keySize);
	p += keySize;
	EncodeForSN(p, (sequenceNumber << 8) | 1);

	SkipList* skipList = (SkipList*)this->head_.AtomicLoad();
	if (skipList->Read(internalKey, value) == 0) {
		// ���û���ڵ�ǰ��Ծ�����ҵ���ת���Ѿ��������ȥ��
		// ����ע����ܵ�ǰ��Ծ�����ҵ�ʱ���Ѿ�ת�������ˣ�Ϊ�˱����ظ��ң�����ȥָ��
		this->compact_->Query(internalKey, value, skipList);
	}
}

void DBImpl::Alter(const char* key, const char* newValue)
{
	// ���￼����һ�£��Ͳ���ƴ���ˣ�Ч�ʿ��Ը���
	// �˺������������ж�keyֵ��¼�Ƿ���ڣ���������д
	// ����ֱ�Ӱ�key�Ͷ����кŴ���SkipList�ж��Ƿ���ڣ�����ֱ�ӵ���Add�ӿ�������¼����
	// �������ܷ�װ�Բ��Ǻܺã���Ϊ����SkipListӦ��ֻ�յ�һ��������internalKey
	uint64_t readSN = this->SN_->GetReadSN();

	SkipList* skipList = (SkipList*)this->head_.AtomicLoad();
	int result = skipList->Contain(key, readSN);
	if (result == -1) return;
	if (result == 0) {
		// ����Query�������ظ���
		if (!this->compact_->Contain(key, readSN, skipList))
			return;
	}	

	// ����˵������key�ļ�¼�������޸�
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
