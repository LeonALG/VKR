#define INITGUID

#include "Plata.h"

#include <windows.h>
#include <math.h>
#include <fstream>
#include <stdio.h>
#include <conio.h>
#include <clocale>
#include <iostream>

#include <iomanip>
#include <objbase.h>

#include "Convert.h"
#include "ADCFile.h"

#include <thread>

#include "C:\Program Files (x86)\LCard\LIBRARY\include\ioctl.h"
#include "C:\Program Files (x86)\LCard\LIBRARY\include\ifc_ldev.h"
#include "C:\Program Files (x86)\LCard\LIBRARY\include\e440cmd.h"

using namespace std;

#define M_FAIL(x,s) do { cout << "FAILED  -> " << x << " ERROR: " << s << endl;  } while(0)
#define M_OK(x,e)   do { cout << "SUCCESS -> " << x << e; } while(0)

typedef IDaqLDevice* (*CREATEFUNCPTR)(ULONG Slot);
CREATEFUNCPTR CreateInstance;

ULONG  pointsize;     // pI->GetParameter(L_POINT_SIZE, &ps) возвращает размер отсчета в байтах (обычно 2, но 791 4 байта)

void* data;
ULONG* sync;

void* fdata = NULL;
HANDLE   hFile = NULL, hMap = NULL;
long     fsize;
HANDLE  hThread = NULL;
ULONG   Tid;
LONG   complete;

USHORT IrqStep = 1024;//777-777%7; // половинка буфера кратная числу каналов или не обязательно кратная
USHORT FIFO = 1024;         //
USHORT pages = 128;  // количество страниц IrqStep в кольцевом буфере PC
USHORT multi = 4;    // - количество половинок кольцевого буфера, которое мы хотим собрать и записать на диск

Plata::Plata()
{

}

HINSTANCE CallCreateInstance(const char* name)
{
    HINSTANCE hComponent = ::LoadLibraryA(name);
    if (hComponent == NULL)
    {
        return 0;
}

    CreateInstance = (CREATEFUNCPTR)::GetProcAddress(hComponent, "CreateInstance");
    if (CreateInstance == NULL)
    {
        return 0;
    }
    return hComponent;
}

Plata::Plata(int argc, char* argv[])
{
    setlocale(LC_CTYPE, "");

#ifdef WIN64
    CallCreateInstance("lcomp64.dll");
#else
    CallCreateInstance("lcomp.dll");
#endif

    cout << ".......... Get IUnknown pointer" << endl;
    LUnknown* pIUnknown = CreateInstance(0);
    if (pIUnknown == NULL)
    {
        cout << "FAILED  -> CreateInstance" << endl;
        ClosePlata();
        return;
    }
    cout << "SUCCESS -> CreateInstance" << endl;

    cout << ".......... Get IDaqLDevice interface" << endl;

    HRESULT hr = pIUnknown->QueryInterface(IID_ILDEV, (void**)&pI);
    if (!SUCCEEDED(hr))
    {
        cout << "FAILED  -> QueryInterface" << endl;
        return;
    }
    cout << "SUCCESS -> QueryInterface" << endl;

    status = pIUnknown->Release();
    M_OK("Release IUnknown", endl);
    cout << ".......... Ref: " << status << endl;

    HANDLE hVxd = pI->OpenLDevice(); // открываем устройство
    if (hVxd == INVALID_HANDLE_VALUE)
    {
        M_FAIL("OpenLDevice", hVxd);
        ClosePlata();
        return;
    }
    else M_OK("OpenLDevice", endl);
    cout << ".......... HANDLE: " << hex << hVxd << endl;

    status = pI->GetSlotParam(&sl);
    if (status != L_SUCCESS)
    {
        M_FAIL("GetSlotParam", status);
        ClosePlata();
        return;
    }
    else M_OK("GetSlotParam", endl);

    cout << ".......... Type    " << sl.BoardType << endl;
    cout << ".......... DSPType " << sl.DSPType << endl;
    cout << ".......... InPipe MTS" << sl.Dma << endl;
    cout << ".......... OutPipe MTS" << sl.DmaDac << endl;

    status = pI->LoadBios(argv[2]); // загружаем биос в модуль
    if ((status != L_SUCCESS) && (status != L_NOTSUPPORTED))
    {
        M_FAIL("LoadBios", status);
        ClosePlata();
        return;
    }
    else M_OK("LoadBios", endl);

    status = pI->PlataTest(); // тестируем успешность загрузки и работоспособность биос
    if (status != L_SUCCESS)
    {
        M_FAIL("PlataTest", status);
        ClosePlata();
        return;
    }
    else M_OK("PlataTest", endl);

    status = pI->ReadPlataDescr(&pd); // считываем данные о конфигурации платы/модуля. 
    // ОБЯЗАТЕЛЬНО ДЕЛАТЬ! (иначе расчеты параметров сбора данных невозможны тк нужна информация о названии модуля и частоте кварца )
    if (status != L_SUCCESS)
    {
        M_FAIL("ReadPlataDescr", status);
        ClosePlata();
        return;
    }
    else M_OK("ReadPlataDescr", endl);

    if (sl.BoardType == E440)
    {
        cout << ".......... SerNum       " << pd.t4.SerNum << endl;
        cout << ".......... BrdName      " << pd.t4.BrdName << endl;
        cout << ".......... Rev          " << pd.t4.Rev << endl;
        cout << ".......... DspType      " << pd.t4.DspType << endl;
        cout << ".......... IsDacPresent " << pd.t4.IsDacPresent << endl;
        cout << ".......... Quartz       " << dec << pd.t4.Quartz << endl;
    }
    else
    {
        cout << "Its not E400...";
    }
}

int Plata::PrepareStats()
{
    cout << ".......... Prepare ADC streaming" << endl;

    DWORD tm = 10000000;
    status = pI->RequestBufferStream(&tm, L_STREAM_ADC);
    if (status != L_SUCCESS)
    {
        M_FAIL("RequestBufferStream(ADC)", status);
        ClosePlata();
        return 1;
    }
    else M_OK("RequestBufferStream(ADC)", endl);
    cout << ".......... Allocated memory size(word): " << tm << endl;

    // настроили параметрыы сбора
    switch (sl.BoardType)
    {
    case E440:
    {
        adcPar.t1.s_Type = L_ADC_PARAM;
        adcPar.t1.AutoInit = 1;
        adcPar.t1.dRate = 50.0;
        adcPar.t1.dKadr = 0;
        adcPar.t1.dScale = 0;
        adcPar.t1.SynchroType = 3; //3
        if (sl.BoardType == E440 || sl.BoardType == E140 || sl.BoardType == E154) adcPar.t1.SynchroType = 0;//0
        adcPar.t1.SynchroSensitivity = 0;
        adcPar.t1.SynchroMode = 0;
        adcPar.t1.AdChannel = 0;
        adcPar.t1.AdPorog = 0;
        adcPar.t1.NCh = 4;
        adcPar.t1.Chn[0] = 0x0;
        /* adcPar.t1.Chn[1] = 0x1;
         adcPar.t1.Chn[2] = 0x2;
         adcPar.t1.Chn[3] = 0x3;*/
        adcPar.t1.FIFO = 1024;
        adcPar.t1.IrqStep = 1024;
        adcPar.t1.Pages = 128;
        if (sl.BoardType == E440 || sl.BoardType == E140 || sl.BoardType == E154)
        {
            adcPar.t1.FIFO = 4096;
            adcPar.t1.IrqStep = 4096;
            adcPar.t1.Pages = 32;
        }
        adcPar.t1.IrqEna = 1;
        adcPar.t1.AdcEna = 1;

        status = pI->FillDAQparameters(&adcPar.t1);
        if (status != L_SUCCESS)
        {
            M_FAIL("FillDAQparameters(ADC)", status);
            ClosePlata();
            return 1;
        }
        else M_OK("FillDAQparameters(ADC)", endl);

        cout << ".......... Buffer size(word):      " << tm << endl;
        cout << ".......... Pages:                  " << adcPar.t1.Pages << endl;
        cout << ".......... IrqStep:                " << adcPar.t1.IrqStep << endl;
        cout << ".......... FIFO:                   " << adcPar.t1.FIFO << endl;
        cout << ".......... Rate:                   " << adcPar.t1.dRate << endl;
        cout << ".......... Kadr:                   " << adcPar.t1.dKadr << endl << endl;

        status = pI->SetParametersStream(&adcPar.t1, &tm, (void**)&::data, (void**)&sync, L_STREAM_ADC);
        if (status != L_SUCCESS)
        {
            M_FAIL("SetParametersStream(ADC)", status);
            ClosePlata();
            return 1;
        }
        else M_OK("SetParametersStream(ADC)", endl);

        cout << ".......... Used buffer size(points): " << tm << endl;
        cout << ".......... Pages:                  " << adcPar.t1.Pages << endl;
        cout << ".......... IrqStep:                " << adcPar.t1.IrqStep << endl;
        cout << ".......... FIFO:                   " << adcPar.t1.FIFO << endl;
        cout << ".......... Rate:                   " << adcPar.t1.dRate << endl;
        cout << ".......... Kadr:                   " << adcPar.t1.dKadr << endl << endl;

        IrqStep = adcPar.t1.IrqStep; // обновили глобальные переменные котрые потом используются в ServiceThread
        pages = adcPar.t1.Pages;

    } break;
    }

    pI->GetParameter(L_POINT_SIZE, &pointsize);
    if (status != L_SUCCESS)
    {
        M_FAIL("GetParameter", status);
        ClosePlata();
        return 1;
    }
    else M_OK("GetParameter", endl);

    cout << ".......... Point size:                   " << pointsize << endl << endl;
    
    cout << "Complete..." << endl;

    return 0;
}

// Поток в котором осуществляется сбор данных
ULONG WINAPI Plata::ServiceThread()
{
    ULONG halfbuffer = IrqStep * pages / 2; // Собираем половинками кольцевого буфера
    ULONG s;
    InterlockedExchange(&s, *sync);
    ULONG fl2, fl1 = fl2 = (s <= halfbuffer) ? 0 : 1;  // Настроили флаги
    int16_t* tmp, * tmp1;
    tmp = new int16_t[halfbuffer];

    //thread* threadAnalysis = NULL;

    //ifstream fin("3.dat", ios::binary | ios::in);
    //// Определяем размер файла. endpos - в байтах, pointCount - в точках.
    //fin.seekg(0, ios::end); //Стать в конец файла
    //streampos endpos = fin.tellg();
    //fin.seekg(0, ios::beg);//Стать на 0-й байт
    //const int64_t CountPoint = endpos / sizeof(int16_t);
    //int16_t* data = new int16_t[CountPoint];
    //// Читаем данные. Шаманизм нужен, т.к. данные читаются "побайтово".
    //fin.read(reinterpret_cast<char*>(data), endpos);
    //fin.close();
    //int sizemass = (endpos / 2) / 65536;
    //cout << "sizemass = " << sizemass << endl;
    //int16_t** dataAll = new int16_t*[sizemass];
    //for (int i = 0; i < sizemass; i++)
    //{
    //    dataAll[i] = new int16_t[halfbuffer];
    //    for (size_t j = 0; j < halfbuffer; j++)
    //    {
    //        dataAll[i][j] = data[i * halfbuffer + j];
    //    }
    //}

    int i = -1;

    /*ifstream fin("2.txt", ios::in);

    double** tmp2txt = new double*[4];
    for (size_t i = 0; i < 4; i++)
    {
        tmp2txt[i] = new double[halfbuffer];
        for (size_t j = 0; j < halfbuffer; j++)
        {
            fin >> tmp2txt[i][j];
        }
    }
    fin.close();
    ofstream fout("TEST2.txt", ios::out);
    for (size_t j = 0; j < halfbuffer; j++)
        fout << tmp2txt[1][j] << endl;
    fout.close();*/
    while (writeFlag)
    {
        /*if (i == (sizemass - 1))
            break;*/
        i++;
        while (fl2 == fl1)
        {
            if (InterlockedCompareExchange(&complete, 3, 3))
                return 0;
            InterlockedExchange(&s, *sync);
            fl2 = (s <= halfbuffer) ? 0 : 1; // Ждем заполнения половинки буфера
        }
        //tmp = ((char*)fdata + (halfbuffer * i) * pointsize);   // Настраиваем указатель в файле
        tmp1 = (int16_t*)((char*) ::data + (halfbuffer * fl1) * pointsize); // Настраиваем указатель в кольцевом буфере
        memcpy_s(tmp, halfbuffer * sizeof(int16_t), tmp1, halfbuffer * pointsize); // Записываем данные в файл
        bool ReWriteOrwrite = i == 0 ? true : false;
        //threadAnalysis = new thread(&CrossMain, halfbuffer, dataAll[i], i, ReWriteOrwrite);
        //threadAnalysis = new thread(&CrossMain, halfbuffer, tmp1, i, ReWriteOrwrite);
        
        fout->FileCopy(tmp, halfbuffer);
        //fout->FileCopyDat(tmp, halfbuffer);

        InterlockedExchange(&s, *sync);
        fl1 = (s <= halfbuffer) ? 0 : 1; // Обновляем флаг  
        Sleep(0); // если собираем медленно то можно и спать больше
    }
    //threadAnalysis->join();
    return 0; // Вышли
}

int Plata::StartADC(std::thread **hThreads, ADCFile* fileout)
{
    fout = fileout;
    cout << ".......... Starting ..." << endl;
    
    fout->FileCreate("data.dat");

    //// Создаем файл 
    //fsize = multi * (pages / 2) * IrqStep; // размер файла
    //cout << "ITS ME Fsize " << fsize << endl;
    //hFile = CreateFileA("data.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);
    //
    //if (hFile == INVALID_HANDLE_VALUE)
    //{
    //    M_FAIL("CreateFile(data.dat)", GetLastError());
    //    ClosePlata();
    //    return 1;
    //}
    //else M_OK("CreateFile(data.dat)", endl);
    //hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, fsize * pointsize, NULL);
    //if (hMap == INVALID_HANDLE_VALUE)
    //{
    //    M_FAIL("CreateFileMapping(data.dat)", GetLastError());
    //    ClosePlata();
    //    return 1;
    //}
    //else M_OK("CreateFileMapping(data.dat)", endl);
    //fdata = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
    //if (fdata == NULL)
    //{
    //    M_FAIL("MapViewOfFile(data.dat)", GetLastError());
    //    ClosePlata();
    //    return 1;
    //}
    //else M_OK("MapViewOfFile(data.dat)", endl);
    //complete = 0;
    //pI->EnableCorrection(); // можно включить коррекцию данных, если она поддерживается модулем

    status = pI->InitStartLDevice(); // Инициализируем внутренние переменные драйвера
    if (status != L_SUCCESS)
    {
        M_FAIL("InitStartLDevice(ADC)", status);
        ClosePlata();
        return 1;
    }
    else M_OK("InitStartLDevice(ADC)", endl);
    //boost::thread hThread(boost::bind(&ServiceThread));
    writeFlag = true;
    (*hThreads) = new std::thread(&Plata::ServiceThread, this);
    //hThread = CreateThread(0, 0x2000, ServiceThread, 0, 0, &Tid); // Создаем и запускаем поток сбора данных 

    status = pI->StartLDevice(); // Запускаем сбор в драйвере
    if (status != L_SUCCESS)
    {
        M_FAIL("StartLDevice(ADC)", status);
        ClosePlata();
        return 1;
    }
    else M_OK("StartLDevice(ADC)", endl);
    cout << endl;

    ////Печатаем индикатор сбора данных
    //while (writeFlag)
    //{
    //    if (writeFlag == false)
    //    {
    //        hThreads.detach();
    //        cout << endl << ".......... Stop." << endl;
    //        break;
    //    }
    //}
    //while (!_kbhit())
    //{
    //    ULONG s;
    //    InterlockedExchange(&s, *sync);
    //    cout << ".......... " << setw(6) << s << "\r";
    //    if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, 0))
    //    {
    //        complete = 1;
    //        break;
    //    }
    //    Sleep(20);
    //}
    //if (!complete)
    //{
    //    cout << endl << ".......... Wait for thread completition..." << endl;
    //    InterlockedBitTestAndSet(&complete, 0); //complete=1
    //    WaitForSingleObject(hThread, INFINITE);
    //}
    
    return 0;
}

int Plata::ClosePlata()
{
    cout << "ADC Complete ..." << endl;
    status = pI->StopLDevice(); // Остановили сбор
    if (status != L_SUCCESS)
    {
        M_FAIL("StopLDevice(ADC)", status);
        return 1;
    }
    else M_OK("StopLDevice(ADC)", endl);

    fout->FileClose();

    return 0;
}

Plata::~Plata()
{
    pI->CloseLDevice();
    pI->Release();
    cout << ".......... Exit." << endl;
}