// 专用于跳表新增节点时随机层数的生成，不知道为什么线程安全
/* 随机数生成用乘同余法（C=0）
int rand(int &seed)
{
seed = (seed * A + C ) % M;
result = seed/M;     // 此行在本程序中不需要
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
		// 经前人测试这一对数据效果较好
		static const size_t M = 2147483647L;   // 2^31-1
		static const uint64_t A = 16807;  // 7^5

		uint64_t tmp = seed_ * A;

		seed_ = (size_t)((tmp >> 31) + (tmp & M));  
		// 上行代码实际意义是 seed_ = tmp % M,可证明相等,如此可提升效率													   
		// 只是 (tmp & M)应该是(tmp & M) % M, 不应该有可能等于M,所以有下面的操作
		if (seed_ > M) {
			seed_ -= M;
		}
		return seed_;
	}

	int RandomHeight(int maxLevel)
	{
		// 简单考虑四叉树的情况，根据论文四叉树最优，即高层节点的数目是低层的1/4
		int height = 0;

	    // 即层数有1/4的几率+1
		while ((Rand() & 3) == 0 && height < maxLevel) {  // 这里 &3==0 即 %4==0,效率更高
			++height;
		}

		return height;
	}
};

#endif // !_MY_DB_RANDOM_H_