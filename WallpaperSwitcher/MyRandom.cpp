#include "pch.h"
#include "MyRandom.h"

void MyRandom::_shuffle()
{
	// 以当前时间置随机数种子
	default_random_engine generator((unsigned int)time(NULL));
	// 创建均匀分布的随机数发生器
	uniform_int_distribution<long long> distribution(0, _nums.size() - 1);
	long long temp;
	long long index;

	// 洗牌算法
	for (long long i = _nums.size() - 1; i > 0; i--)
	{
		do
		{
			// 产生下一个随机数
			index = distribution(generator);
		} while (index >= i);

		temp = _nums[i];
		_nums[i] = _nums[index];
		_nums[index] = temp;
	}
}

MyRandom::MyRandom()
{
	_isSetRange = false;
	setRange(0, 1);
}

MyRandom::MyRandom(long long leftRange, long long rightRange)
{
	_isSetRange = true;
	setRange(leftRange, rightRange);
	_shuffle();
}

long long MyRandom::randNumber()
{
	if (!_isSetRange)
	{
		throw range_error("随机数范围非法");
	}

	long long num;

	// 如果上次生成的随机数还没有用完
	if (_nums.size() != 0)
	{
		num = _nums[_nums.size() - 1];
		_nums.pop_back();
		return num;
	}
	else
	{
		setRange(_leftRange, _rightRange);
		_shuffle();
		return randNumber();
	}
}

void MyRandom::setRange(long long leftRange, long long rightRange)
{
	if (leftRange > rightRange)
	{
		throw range_error("随机数范围非法");
	}
	else
	{
		_isSetRange = true;
		_leftRange = leftRange;
		_rightRange = rightRange;
		
		_nums.clear();

		// 像数组中顺序添加范围内所有的数值，之后在用洗牌算法打乱数组
		for (long long int i = leftRange; i <= rightRange; i++)
		{
			_nums.push_back(i);
		}

		_shuffle();
	}
}
