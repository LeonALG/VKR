#pragma once

#include <windows.h>
#include <string>
#include "Convert.h"

class ADCFile
{
	std::string name;
	HANDLE hFile;

	uint32_t size;
	int16_t* dataF;

	HANDLE Event;

	ConvertADC* convert;

public:
	ADCFile();
	ADCFile(HANDLE Event, ConvertADC* convert);
	bool FileCreate(std::string name);

	bool FileCopy(int16_t* buffer, uint32_t size);

	bool FileClose();
	void FileWrite();
	~ADCFile();

	//bool FileCopyDat(int16_t* buffer, uint32_t size);
};