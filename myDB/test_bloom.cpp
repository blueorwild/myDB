#include "bloom.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

Bloom bloom;
int a = 0;

void insert() {
	string key;
	ostringstream key_filename("");
	ifstream in_key;
	for (int i = 1; i <= 1; i++) {
		key_filename.str("");
		key_filename << "./data/1/key" << i << ".txt";
		in_key.open(key_filename.str());
		for (int j = 0; j < 10000; j++) {
			in_key >> key;
			bloom.AddKey(key.c_str());
		}
		in_key.close();
	}
}

void query(int i) {
	string key;
	ostringstream key_filename("");
	ifstream in_key;
	//for (int i = 16; i <= 21; i++) {
		key_filename.str("");
		key_filename << "./data/1/value" << i << ".txt";
		in_key.open(key_filename.str());
		for (int j = 0; j < 10000; j++) {
			in_key >> key;
			if(bloom.QueryKey(key.c_str())) a++;
		}
		in_key.close();
	//}
}

int main() {
	insert();
	for (int i = 2; i <= 100; i++) {
		a = 0;
		query(i);
		cout << a << endl;
	}
	system("pause");
	return 0;
}