#include <iostream>
#include "Plata.h"
#include "Conio.h"
#include "ADCFile.h"
#include "Convert.h"
#include <thread>

using namespace std;


int main(int argc, char* argv[])
{
    Plata* plata = new Plata(argc, argv);

    HANDLE Event = CreateEvent(NULL, TRUE, FALSE, NULL);

    ConvertADC* convert = new ConvertADC(Event);
    ADCFile* file = new ADCFile(Event, convert);

    std::thread* plataThread = NULL;
    std::thread* fileThread = new std::thread(&ADCFile::FileWrite, file);
    std::thread* convertThread = new std::thread(&ConvertADC::CrossMain, convert);

    int flag = -1;
    plata->PrepareStats();
    cout << endl << "Press '1' to Start ADC; '2' to Pause ADC; '0' to Exit." << endl;
    while (true)
    {
        char choose = _getch();
        switch (choose)
        {
            case '1':
            {
                if (flag != 1)
                {
                    flag = 1;
                    plata->StartADC(&plataThread, file);
                }
                break;
            }
            case '2':
            {
                if (flag == 1)
                {
                    plata->writeFlag = false;
                    plataThread->join();
                    plata->ClosePlata();
                    delete plataThread;
                    flag = 2;
                    cout << endl << "Press '1' to Start ADC; '2' to Pause ADC; '0' to Exit." << endl;
                }
                break;
            }
            case '0':
            {
                if (flag == 1)
                {
                    plata->writeFlag = false;
                    plataThread->join();
                    plata->ClosePlata();
                    delete plataThread;
                }
                flag = 0;
                fileThread->detach();
                convertThread->detach();
                delete fileThread;
                delete convertThread;
                delete plata;
                return 0;
            }
        }
    }
    return 0;
}