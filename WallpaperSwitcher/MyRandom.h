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
	������ࣻ
	��ָ����Χ����n�Ŀ��ܵ������

	����ȡn���������ÿ�����־�֮����һ��
	�������ȡ2n���������ǰn�ε���������кͺ�n�ε���������м���������ͬ

	ע�⣺�����Ե�ǰʱ��Ϊ��������ӣ������һ���ڴ���ȡ�����������ô�ڴ�ȡ��n+1һ������ʼ����ʧȥ�����
*/

class MyRandom
{
private:
	// ��¼�Ƿ������˷�Χ��δ���÷�Χ��ȡ����������쳣
	bool _isSetRange;
	// ��������������䣬�����߽�
	_size _leftRange, _rightRange;
	// �洢������������
	vector<_size> _nums;
	// ϴ���㷨
	void _shuffle();
public:
	MyRandom();
	MyRandom(_size leftRange, _size rightRange);
	// ��ȡ��һ�������
	_size randNumber();
	// ������������䣬�����߽�
	void setRange(_size leftRange, _size rightRange);
};

