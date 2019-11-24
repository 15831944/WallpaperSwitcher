#pragma once
#ifdef _WIN64
typedef INT64 _size;
#else
typedef INT32 _size;
#endif


#include <time.h>
#include <random>
#include <stdexcept>
#include <vector>

using std::uniform_int_distribution;
using std::default_random_engine;
using std::range_error;
using std::vector;

/*
	随机数类；
	设指定范围内由n的可能的随机数

	连续取n次随机数，每个数字均之出现一次
	而且如果取2n次随机数，前n次的随机数序列和后n次的随机数序列几乎不会相同

	注意：该类以当前时间为随机数种子，如果在一秒内大量取出随机数，那么在从取第n+1一个数开始，将失去随机性
*/

class MyRandom
{
private:
	// 记录是否设置了范围，未设置范围就取随机数会抛异常
	bool _isSetRange;
	// 随机数的左右区间，包含边界
	_size _leftRange, _rightRange;
	// 存储随机打算的数字
	vector<_size> _nums;
	// 洗牌算法
	void _shuffle();
public:
	MyRandom();
	MyRandom(_size leftRange, _size rightRange);
	// 获取下一个随机数
	_size randNumber();
	// 设置随机数区间，包含边界
	void setRange(_size leftRange, _size rightRange);
};

