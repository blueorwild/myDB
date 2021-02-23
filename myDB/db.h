#ifndef _MY_DB_H_
#define _MY_DB_H_

#include <string>

class SnapShot {
protected:
	virtual ~SnapShot();   // ����������������������ָ��ָ�����������ȷ����
};

class DB {
private:
	
public:
	DB() {};
	virtual ~DB() {};

	// ���ڴ�ģ�ⴴ��һ�������ݿ⡱,����ʼ�������Ϣ
	static void CreateDB(DB** pDB);
	
	// ɾ�����������ݿ⣨��Ҫ�������ڴ棩
	virtual void DeleteDB() = 0;

	// ����һ��key-value��¼ 
	virtual void Add(const char* key, const char* value) = 0;

	// ɾ��ĳkeyֵ�ļ�¼��ʵ��������һ������Ϊɾ����valueΪNULL��key��¼��
	virtual void Delete(const char* key) = 0;

	// ����keyֵ����value�����snapshotΪ�գ����ڵ�ǰ���µĶ����кţ��������snapshot
	virtual void Query(const SnapShot* snapshot, const char* key, std::string* value) = 0;

	// �޸�һ��key��value��¼��ʵ�ʾ�������������������������ԭkey��¼�򲻽����κβ���
	virtual void Alter(const char* key, const char* newValue) = 0;

	// ��ȡ���½���һ��snapshot
	virtual const SnapShot* GetSnapShot() = 0;

	// ɾ��ĳ��snapshot
	virtual void DeleteSnapShot(const SnapShot* snapshot) = 0;
};


#endif // !_MY_DB_H_