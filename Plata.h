#pragma once

#include <windows.h>
#include <math.h>

#include <iomanip>
#include <objbase.h>

#include "Convert.h"
#include "ADCFile.h"

#include "C:\Program Files (x86)\LCard\LIBRARY\include\ioctl.h"
#include "C:\Program Files (x86)\LCard\LIBRARY\include\ifc_ldev.h"
#include "C:\Program Files (x86)\LCard\LIBRARY\include\e440cmd.h"
//#include <boost/thread.hpp>

#include <thread>

class Plata
{
    ADC_PAR adcPar;
    PLATA_DESCR_U2 pd;
    SLOT_PAR sl;
    ULONG status;
    
    IDaqLDevice* pI;

    ADCFile* fout;

    //HINSTANCE CallCreateInstance(const char* name);
public:
    bool writeFlag;

    Plata();
    Plata(int argc, char* argv[]);
    ~Plata();

    ULONG WINAPI ServiceThread();

    int PrepareStats();

    int StartADC(std::thread **hThreads, ADCFile* fileout);

    int ClosePlata();

};