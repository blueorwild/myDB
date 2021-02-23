// ר�������������ڵ�ʱ������������ɣ���֪��Ϊʲô�̰߳�ȫ
/* ����������ó�ͬ�෨��C=0��
int rand(int &seed)
{
seed = (seed * A + C ) % M;
result = seed/M;     // �����ڱ������в���Ҫ
}
*/

#ifndef _MY_DB_RANDOM_H_
#define _MY_DB_RANDOM_H_

class Random {

private:
	size_t seed_;

public:
	Random(size_t s) :seed_(s) {};

	size_t Rand() {
		// ��ǰ�˲�����һ������Ч���Ϻ�
		static const size_t M = 2147483647L;   // 2^31-1
		static const uint64_t A = 16807;  // 7^5

		uint64_t tmp = seed_ * A;

		seed_ = (size_t)((tmp >> 31) + (tmp & M));  
		// ���д���ʵ�������� seed_ = tmp % M,��֤�����,��˿�����Ч��													   
		// ֻ�� (tmp & M)Ӧ����(tmp & M) % M, ��Ӧ���п��ܵ���M,����������Ĳ���
		if (seed_ > M) {
			seed_ -= M;
		}
		return seed_;
	}

	int RandomHeight(int maxLevel)
	{
		// �򵥿����Ĳ�������������������Ĳ������ţ����߲�ڵ����Ŀ�ǵͲ��1/4
		int height = 0;

	    // ��������1/4�ļ���+1
		while ((Rand() & 3) == 0 && height < maxLevel) {  // ���� &3==0 �� %4==0,Ч�ʸ���
			++height;
		}

		return height;
	}
};

#endif // !_MY_DB_RANDOM_H_