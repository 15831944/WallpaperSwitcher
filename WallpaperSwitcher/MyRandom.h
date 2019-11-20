#pragma once
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
	long long _leftRange, _rightRange;
	// �洢������������
	vector<long long> _nums;
	// ϴ���㷨
	void _shuffle();
public:
	MyRandom();
	MyRandom(long long leftRange, long long rightRange);
	// ��ȡ��һ�������
	long long randNumber();
	// ������������䣬�����߽�
	void setRange(long long leftRange, long long rightRange);
};

