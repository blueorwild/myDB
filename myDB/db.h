#ifndef _MY_DB_H_
#define _MY_DB_H_

#include <string>

class SnapShot {
protected:
	virtual ~SnapShot();   // 定义虚拟析构函数，父类指针指向的子类能正确析构
};

class DB {
private:
	
public:
	DB() {};
	virtual ~DB() {};

	// 在内存模拟创建一个“数据库”,并初始化相关信息
	static void CreateDB(DB** pDB);
	
	// 删除建立的数据库（主要是清理内存）
	virtual void DeleteDB() = 0;

	// 新增一条key-value记录 
	virtual void Add(const char* key, const char* value) = 0;

	// 删除某key值的记录（实际是新增一条类型为删除，value为NULL的key记录）
	virtual void Delete(const char* key) = 0;

	// 根据key值查找value，如果snapshot为空，基于当前最新的读序列号，否则根据snapshot
	virtual void Query(const SnapShot* snapshot, const char* key, std::string* value) = 0;

	// 修改一条key的value记录，实际就是是新增，区别是若不存在原key记录则不进行任何操作
	virtual void Alter(const char* key, const char* newValue) = 0;

	// 获取（新建）一个snapshot
	virtual const SnapShot* GetSnapShot() = 0;

	// 删除某个snapshot
	virtual void DeleteSnapShot(const SnapShot* snapshot) = 0;
};


#endif // !_MY_DB_H_