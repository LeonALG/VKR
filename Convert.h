#pragma once

#include <string>

class ConvertADC
{
	int16_t* dataConv;
	ULONG size;

	double* MainPoint;
public:

	HANDLE Event;

	ConvertADC();

	ConvertADC(HANDLE Event);

	~ConvertADC();

	bool SetData(int16_t* data, ULONG size);

	bool CrossMain();

	bool FindNoise(int j, int i, int CountPoint);

	bool FindMaxPoint(int j, int i, int CountPoint);

	bool FindImpact(int j, int i, int CountPoint);

	bool FindPit(int j, int i, int CountPoint);
};