#include "ADCFile.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

//std::ifstream fin("2.dat", std::ios::binary | std::ios::in);

ADCFile::ADCFile()
{
    name = "";
    hFile = NULL;
    dataF = NULL;
    convert = NULL;
    size = 0;
}

ADCFile::ADCFile(HANDLE Event, ConvertADC* convert)
{
    name = "";
    hFile = NULL;
    dataF = NULL;
    size = 0;
    this->Event = Event;
    this->convert = convert;
}

bool ADCFile::FileCreate(std::string name)
{
	this->name = name;
	hFile = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error create file" << std::endl;
        return false;
    }
    return true;
}

bool ADCFile::FileClose()
{
    if (hFile) CloseHandle(hFile);
    {
        std::cout << "Close file " << name.c_str() << std::endl;
        return true;
    }
    return false;
}

//bool ADCFile::FileCopyDat(int16_t* buffer, uint32_t size)
//{
//    if (this->size != size)
//    {
//        this->size = size;
//        if (dataF != NULL)
//        {
//            delete[] dataF;
//            dataF = NULL;
//        }
//        if (this->size > 0)
//        {
//            dataF = new int16_t[this->size];
//        }
//        else
//            return false;
//    }
//    
//    // Определяем размер файла. endpos - в байтах, pointCount - в точках.
//    int16_t* data = new int16_t[size * sizeof(int16_t)];
//    // Читаем данные. Шаманизм нужен, т.к. данные читаются "побайтово".
//    fin.read(reinterpret_cast<char*>(data), size * sizeof(int16_t));
//    
//    convert->SetData(data, this->size);
//
//    /*SetEvent(convert->Event);
//    ResetEvent(convert->Event);*/
//
//    SetEvent(Event);
//    ResetEvent(Event);
//
//    return true;
//}

bool ADCFile::FileCopy(int16_t* buffer, uint32_t size)
{
    if (this->size != size)
    {
        this->size = size;
        if (dataF != NULL)
        {
            delete[] dataF;
            dataF = NULL;
        }
        if (this->size > 0)
        {
            dataF = new int16_t[this->size];
        }
        else
            return false;
    }
    memcpy(dataF, buffer, this->size * sizeof(int16_t));

    convert->SetData(dataF, this->size);

    /*SetEvent(convert->Event);
    ResetEvent(convert->Event);*/

    SetEvent(Event);
    ResetEvent(Event);

    return true;
}

void ADCFile::FileWrite()
{
    //const double phys1 = 0.0;
    //const double phys2 = 36.03;
    //const double code1 = -90.6653;
    //const double code2 = -15552.47;
    //const double k = (phys2 - phys1) / (code2 - code1);
    //std::ofstream fout("TESTDATA.txt", std::ios::out | std::ios::app);
    //fout << std::setprecision(12);
    //for (ULONG i = 0; i < size/2; i++)
    //{
    //    fout << (k* ((buffer)[i] - code1) + phys1) << std::endl;
    //}
    while (true)
    {
        DWORD waitRes = WaitForSingleObject(Event, INFINITE);
        DWORD fileWrite;
        WriteFile(hFile, dataF, size * sizeof(int16_t), &fileWrite, NULL);
        FlushFileBuffers(hFile);
        std::cout << "Write..." << std::endl << "size = " << size << std::endl;
    }
}

ADCFile::~ADCFile()
{
    if (hFile) CloseHandle(hFile);

    //fin.close();
}