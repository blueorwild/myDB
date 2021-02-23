#include "db.h"
#include <iostream>

using namespace std;

int main()
{
	DB* p = NULL;
	p->CreateDB(&p);

	p->Add("hello", "world");
	cout << "�����¼��" << "hello world" <<  endl;
	const SnapShot* s = p->GetSnapShot();
	cout << "���ɿ���" << endl;
	
	p->Add("hello", "world2");
	cout << "�����¼��" << "hello world2" << endl;

	string value;
	p->Query(s, "hello", &value);
	cout << "��ȡ���գ�" << value << endl;

	p->Query(NULL, "hello", &value);
	cout << "��ȡ���£�" << value << endl;

	p->DeleteSnapShot(s);
	cout << "ɾ������" << endl;
	p->Query(s, "hello", &value);
	cout << "��ȡ���գ�" << value << endl;

	cin.get();
	return 0;
}