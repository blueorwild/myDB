// snapshotʵ����ֻ��һ��64λ(56λ��Ч)�����֣�����ĳ�����кţ�����һ��˫�������
// Ҳ���õ���ģʽ

#ifndef _MY_DB_SNAPSHOT_H_
#define _MY_DB_SNAPSHOT_H_

#include "db.h"

class SnapShotList;

// ����Ҳ������SkipList�Ľڵ�һ�����ó�һ���ڲ��Ľṹ��
struct SnapShotImpl : public SnapShot {
private:
	friend class SnapShotList;

	SnapShotImpl* prev_;
	SnapShotImpl* next_;

public:
	uint64_t number_;

};

class SnapShotList {
private:
	static SnapShotList* pSnapShot_;  // �ⲿ����SnapshotList��Ψһ�ӿ�

	SnapShotImpl *head_;              // SnapshotList�ڲ�������ͷ���
	SnapShotList() {
		this->head_ = new SnapShotImpl;
		head_->prev_ = head_;
		head_->next_ = head_;
	}

public:
	static SnapShotList* Initial() {
		return pSnapShot_;
	}

	// �жϿ������Ƿ�Ϊ��
	bool empty();

	// ��ȡ���ϵ�snapshot
	SnapShotImpl* oldest();

	// ��ȡ���µ�snapshot
	SnapShotImpl* newest();

	// �����µ�snapshot
	SnapShotImpl* New(uint64_t seq);

	// ɾ��ĳ��snapshot
	void Delete(const SnapShotImpl* s);

};

#endif  //  !_MY_DB_SNAPSHOT_H_

