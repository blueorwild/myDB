#include <assert.h>

#include "snapshot.h"

SnapShotList* SnapShotList::pSnapShot_ = new SnapShotList();

bool SnapShotList::empty()
{ 
	return this->head_->next_ == head_; 
}

// ��ȡ���ϵ�snapshot
SnapShotImpl* SnapShotList::oldest()
{
	assert(!empty());
	return this->head_->next_;
}

// ��ȡ���µ�snapshot
SnapShotImpl* SnapShotList::newest()
{
	assert(!empty());
	return this->head_->prev_;
}

// �����µ�snapshot
SnapShotImpl* SnapShotList::New(uint64_t seq) 
{
	SnapShotImpl* s = new SnapShotImpl;
	s->number_ = seq;
	s->next_ = this->head_;
	s->prev_ = this->head_->prev_;
	s->prev_->next_ = s;
	s->next_->prev_ = s;
	return s;
}

// ɾ��ĳ��snapshot
void SnapShotList::Delete(const SnapShotImpl* s) 
{
	s->prev_->next_ = s->next_;
	s->next_->prev_ = s->prev_;
	delete s;
}
