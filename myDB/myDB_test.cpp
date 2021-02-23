#include "db.h"
#include <iostream>

using namespace std;

int main()
{
	DB* p = NULL;
	p->CreateDB(&p);

	p->Add("hello", "world");
	cout << "插入记录：" << "hello world" <<  endl;
	const SnapShot* s = p->GetSnapShot();
	cout << "生成快照" << endl;
	
	p->Add("hello", "world2");
	cout << "插入记录：" << "hello world2" << endl;

	string value;
	p->Query(s, "hello", &value);
	cout << "读取快照：" << value << endl;

	p->Query(NULL, "hello", &value);
	cout << "读取最新：" << value << endl;

	p->DeleteSnapShot(s);
	cout << "删除快照" << endl;
	p->Query(s, "hello", &value);
	cout << "读取快照：" << value << endl;

	cin.get();
	return 0;
}