#include "pch.h"
#include "MyRandom.h"

void MyRandom::_shuffle()
{
	// �Ե�ǰʱ�������������
	default_random_engine generator((unsigned int)time(NULL));
	// �������ȷֲ��������������
	uniform_int_distribution<_size> distribution(0, _nums.size() - 1);
	_size temp;
	_size index;

	// ϴ���㷨
	for (_size i = _nums.size() - 1; i > 0; i--)
	{
		do
		{
			// ������һ�������
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

MyRandom::MyRandom(_size leftRange, _size rightRange)
{
	_isSetRange = true;
	setRange(leftRange, rightRange);
	_shuffle();
}

_size MyRandom::randNumber()
{
	if (!_isSetRange)
	{
		throw range_error("�������Χ�Ƿ�");
	}

	_size num;

	// ����ϴ����ɵ��������û������
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

void MyRandom::setRange(_size leftRange, _size rightRange)
{
	if (leftRange > rightRange)
	{
		throw range_error("�������Χ�Ƿ�");
	}
	else
	{
		_isSetRange = true;
		_leftRange = leftRange;
		_rightRange = rightRange;
		
		_nums.clear();

		// ��������˳����ӷ�Χ�����е���ֵ��֮������ϴ���㷨��������
		for (_size i = leftRange; i <= rightRange; i++)
		{
			_nums.push_back(i);
		}

		_shuffle();
	}
}
