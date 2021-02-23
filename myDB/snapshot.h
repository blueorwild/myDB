// snapshot实际上只是一个64位(56位有效)的数字，代表某个序列号，放在一个双向链表里。
// 也采用单例模式

#ifndef _MY_DB_SNAPSHOT_H_
#define _MY_DB_SNAPSHOT_H_

#include "db.h"

class SnapShotList;

// 这里也可以像SkipList的节点一样设置成一个内部的结构体
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
	static SnapShotList* pSnapShot_;  // 外部访问SnapshotList的唯一接口

	SnapShotImpl *head_;              // SnapshotList内部真正的头结点
	SnapShotList() {
		this->head_ = new SnapShotImpl;
		head_->prev_ = head_;
		head_->next_ = head_;
	}

public:
	static SnapShotList* Initial() {
		return pSnapShot_;
	}

	// 判断快照链是否为空
	bool empty();

	// 获取最老的snapshot
	SnapShotImpl* oldest();

	// 获取最新的snapshot
	SnapShotImpl* newest();

	// 创建新的snapshot
	SnapShotImpl* New(uint64_t seq);

	// 删除某个snapshot
	void Delete(const SnapShotImpl* s);

};

#endif  //  !_MY_DB_SNAPSHOT_H_

