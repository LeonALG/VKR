//==============================================================================
// ��������� �.�., 17.01.2021 https://www.youtube.com/watch?v=ca1fwd1LEDw&feature=youtu.be
//==============================================================================

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>

#include <windows.h>
#include "Convert.h"

using namespace std;

// ��������� ������ ��� ������� �����.
const double phys1 = 0.0;
const double phys2 = 36.03;
const double code1 = -90.6653;
const double code2 = -15552.47;
const double k = (phys2 - phys1) / (code2 - code1);

// ���� ������ ���������������� ����� ��� ��������� � �������
const char* Result_fileNameOut = "Result.txt"; // ���� � ������������ ������

/*----------------------------------------------------------------------------------*/
/*�������������� ������*/
double FuturePoint = 0.08; // �������� ������� ����� ��� ������ �������� ����� (������ ��)
int Primary_Pit_size = 5; // ��������� �������� ������� ������� ��� 
double Primary_Impact_size = 0.3; // ��������� �������� ������� ������� ����
int Primary_Noise_size = 40; // ��������� �������� ������� ������� ���
int FreeFallSensitivityLimit = 0; // ������� ���������������� ���������� �������
//double Multiplier_Comparison_Max_Value_For_Short_fall = 0.6; // ����. ����� ������, ��� ������ � ��������� �������� (�����)
double Multiplier_Comparison_Max_Value_For_Long_fall = 0.85; // ����. ����� ������, ��� ������ � ������� �������� (�����)
double Noise_min_value = -0.7; // ����������� �������� ���������, ����� ���������� ���
double Noise_max_value = 1.0;  // ������������ �������� ���������, ����� ���������� ���

int Pit_size = 1000; // ������������ ���������� ������ ������ ����� ��� ����������� ������� ��� ���
int Impact_size = 30; // ������������ ���������� ������ ������ ����� ��� ����������� ������� ��� ����
int Noise_size = 40; // ������������ ���������� ������ ������ ����� ��� ����������� ������� ��� ���
/*--------------------*/

bool flag_negative_area = false;  // ���������� ��������� � ������������� ������� (�� �� ��������, ��� �� ����� ���)
bool FreeFallMoment = false; //������� ������� ���������� ������� (����� "���")

bool flag_positive_area = false; // ���������� ��������� � ������������� ������� (�� �� ��������, ��� �� ����� ����)
bool Impact_In_Progress = false; // ���� �� ��, ��� ���� ��������� � �������� (���� ��� ����� �������)

bool flag_area_Noise = false; // ���������� ��������� ��������� (�������� � ���)
bool flag_Noise_moment = false; // ���������� ��������� � ������� ���� (�� �� ��������, ��� �� ����� ���)

ULONG j; // �������
int StartFreeFall = 0; // ����� ������ ���������� �������
int CountFreeFall = 0; // ���������� ��������� � ������������� �������
int StartNoise = 0; // ����� ������ ����
int CountNoise = 0; // ���������� ��������� � ������� ����

int Count_Moment_Of_Impact = 0; // ���������� ����� ����� ������� �����
int Starting_Point_Impact = 0; // ����� �����
int Number_Output = 0; // ����� ������

double Max_Value = 0.0; // ������������� �������� �����
int Max_Value_coord = 0; // ������ ������������ ����� � �������
bool Max_Value_flag = false; // ���� ���������� ������ �������� �����
bool Max_Find_Complete = false; // ����, ��� ������� �������� �����
double EditFrequencyValue = 20; // ���� ������������� �������, �� ������� ��� ������� ����

const int Value_after_�omma = 12; //���������� ������� ����� �������
ofstream* Result_Output_TXT = NULL; // ���� ������ �����������

int half = 0;

/*----------------------------------------------------------------------------------*/

ConvertADC::ConvertADC()
{
    MainPoint = NULL;
    dataConv = NULL;
    size = 0;
}

ConvertADC::ConvertADC(HANDLE Event)
{
    MainPoint = NULL;
    dataConv = NULL;
    size = 0;
    this->Event = Event;
}

ConvertADC::~ConvertADC()
{

}

bool ConvertADC::SetData(int16_t* data, ULONG size)
{
    dataConv = data;
    this->size = size;
    return true;
}

bool ConvertADC::FindPit(int j, int i, int CountPoint)
{
    // ���� ��������� ������ ������� ����������������
    if (MainPoint[j] < FreeFallSensitivityLimit) {
        // ���� �� ������� � ������������� ��� � �� �������� ���, ����� 
        if ((flag_negative_area == false) and (FreeFallMoment == false)) {
            // ���������� ��������� ����� ������ ���
            // ������ ���� �� ��, ��� ��������� � ������������� �������
            StartFreeFall = j;
            half = i;
            flag_negative_area = true;
            CountFreeFall = 0;
        }
    }
    // ���� ��������� ������ ������� ���������������� ���������� �������
    // � ���� ���������������� ���� � ���
    else if (flag_negative_area == true)
    {
        // �� ��� �� � ���  
        flag_negative_area = false;
        CountFreeFall = 0;
    };

    // ���� ����� ������������� �������, ����� ������� ���������� ����� ��� ����������� ���
    if (flag_negative_area == true)
        CountFreeFall++;

    // ���� ���������� ������ ������ ����� � ������������� ������� ������ 1000 �
    // ���� �� ��� �� ������� � ���
    if ((CountFreeFall > Pit_size) and (FreeFallMoment == false)) {
        // ������������� ������� ���
        // ����� ���
        // �������� ���������� ������ ������ �����
        flag_negative_area = false;
        FreeFallMoment = true;
        Number_Output++;
        CountFreeFall = 0;
        cout << "\n[" << Number_Output << "]" << endl;
        *Result_Output_TXT << "[" << Number_Output << "]" << endl;
        cout << "Pit =  " << StartFreeFall + (half * CountPoint) << endl;
        *Result_Output_TXT << "Pit =  " << StartFreeFall + (half * CountPoint) << endl;
        // ���� ��� ��� ���, �� ���� ���
        if (flag_Noise_moment == true)
            flag_Noise_moment = false;
        return true;
    };
    return false;
}

bool ConvertADC::FindImpact(int j, int i, int CountPoint)
{
    // ���� ��������� ������ ������� ����������������
    if (MainPoint[j] > FreeFallSensitivityLimit)
    {
        // ���� �� ���� ��������� � ������������� �������
        if (flag_positive_area == false)
        {
            // �� ����� ������������� �������, ���������� �����, ��� ��� �������
            flag_positive_area = true;
            Count_Moment_Of_Impact = 0;
            Starting_Point_Impact = j;
            half = i;
        }
    }
    // ���� ������������� ������� ��� ����
    else if (flag_positive_area == true)
    {
        // �������� ��
        flag_positive_area = false;
        Count_Moment_Of_Impact = 0;
    }

    // ���� �� ����� ������������� �������, ������� ���������� ����� � ���, ��� ����������� ����� 
    if (flag_positive_area == true)
        Count_Moment_Of_Impact++;

    // ���� ���������� ������ ����� ������������� ������� ������ ���, ������� ���������� ��� �����������
    // ����� ��� ���� � ����� ��� �� ����
    if ((Count_Moment_Of_Impact > Impact_size) and (Impact_In_Progress == false)) {
        // �������� ������������� �������
        flag_positive_area = false;
        // ���������� ����
        Impact_In_Progress = true;
        // ������ ���������� ������� ��� �� ����� = false
        FreeFallMoment = false;
        // �������� ���������� ������ ������ �����
        Count_Moment_Of_Impact = 0;
        // ����������� ������������� �������� ����� �������� ����� ������ ����� (y)
        Max_Value = MainPoint[j - Impact_size];
        // ����������� ������������� �������� ����� �������� ����� ������ ����� (x)
        Max_Value_coord = j - Impact_size;
        // �����
        cout << "Point of impact = " << Starting_Point_Impact + (half * CountPoint) << endl;
        *Result_Output_TXT << "Point of impact = " << Starting_Point_Impact + (half * CountPoint) << endl;
        return true;
    };
    return false;
}

bool ConvertADC::FindMaxPoint(int j, int i, int CountPoint)
{
    // ���� ������ � ����� ����� ����� � ��������� ������ 1.5
    if ((Max_Value * Multiplier_Comparison_Max_Value_For_Long_fall > MainPoint[j + static_cast<int>(round(FuturePoint * EditFrequencyValue))]) &&
        (MainPoint[j] > 1.5))
    {
        // ���� �� ����� �������� �� ���������� ������
        while (MainPoint[Max_Value_coord] <= MainPoint[Max_Value_coord + 1])
            Max_Value_coord++;
        Max_Find_Complete = true;
        // �����
        cout << "Point of maximum = " << Max_Value_coord + (half * CountPoint) << endl;
        cout << "Value of maximum = " << setprecision(Value_after_�omma) << MainPoint[Max_Value_coord] << endl;
        *Result_Output_TXT << "Point of maximum = " << Max_Value_coord + (half * CountPoint) << endl;
        *Result_Output_TXT << "Value of maximum = " << setprecision(Value_after_�omma) << MainPoint[Max_Value_coord] << endl;
        Impact_In_Progress = false;
        return true;
    }
    // ���� ������� �������� ������ �������� ���������, �� ����� ��������
    else if (MainPoint[j] > Max_Value)
    {
        Max_Value = MainPoint[j];
        Max_Value_coord = j;
    }
    return false;
}

bool ConvertADC::FindNoise(int j, int i, int CountPoint)
{
    // ���� ������� ��������� ��������� � ���������� ����������� ����
    if ((MainPoint[j] > Noise_min_value) and (MainPoint[j] < Noise_max_value)) {
        // ���� ���� ��� �� ���� � ��� �� �������� � ������� ����
        if (flag_area_Noise == false) {
            // � ������� ���� ����� � ��������� ���������� ������ ����
            flag_area_Noise = true;
            StartNoise = j;
            half = i;
            CountNoise = 0;
        };
    }
    // ���� ������� ��������� ��������� ��� ����������
    // � ���� ���� ������� ����
    else if (flag_area_Noise == true)
    {
        // ��������
        CountNoise = 0;
        flag_area_Noise = false;
    }

    // ���� ���� ������� ������� ����, ������� ���������� ����� � ���, ��� ����������� ����
    if (flag_area_Noise == true)
        CountNoise++;

    // ���� ���������� ������ ������ ����� ������ ����, ������� ���������� 
    // ��� ����������� ���� � ��� ����������
    if ((CountNoise > Noise_size) and (flag_Noise_moment == false)) {
        // ������ ���� �� ���������� �������� ����� false
        Max_Find_Complete = false;
        // ���� �� ���������� ���� true
        flag_Noise_moment = true;
        // ������ �� ���� ���`
        flag_area_Noise = false;
        CountNoise = 0;
        cout << "End of hesitation = " << StartNoise + (half * CountPoint) << endl;
        *Result_Output_TXT << "End of hesitation = " << StartNoise + (half * CountPoint) << endl;
        return true;
    };
    return false;
}

bool ConvertADC::CrossMain()
{
    Result_Output_TXT = new ofstream(Result_fileNameOut, ios::out | ios::trunc);
    
    ofstream fout("TestACP.txt", ios::out | ios::trunc);
    fout << setprecision(12);
    int i = 0;
    while (true)
    {
        DWORD waitRes = WaitForSingleObject(Event, INFINITE);
        if (MainPoint != NULL)
        {
            delete[] MainPoint;
            MainPoint = new double[size];
        }
        else
        {
            MainPoint = new double[size];
        }
        for (ULONG i = 0; i < size; i++)
        {
            MainPoint[i] = (k * ((dataConv)[i] - code1) + phys1);
            fout << MainPoint[i] << endl;
        }

        EditFrequencyValue = 50;
        /*cout << "Enter the frequency at which the unit operates (in kHz): ";
        cin >> EditFrequencyValue;*/
        Pit_size = static_cast<int>(Primary_Pit_size * EditFrequencyValue);
        Impact_size = static_cast<int>(Primary_Impact_size * EditFrequencyValue);
        Noise_size = static_cast<int>(Primary_Noise_size * EditFrequencyValue);

        for (j = 0; j < size; j++)
        {
            if ((flag_Noise_moment == true) or (Number_Output == 0))
                //����� ���
                FindPit(j, i, size);
            else if (FreeFallMoment == true)
                //����� �����
                FindImpact(j, i, size);
            else if (Impact_In_Progress == true)
                //����� �������� �����
                FindMaxPoint(j, i, size);
            else if (Max_Find_Complete == true)
                //����� ����
                FindNoise(j, i, size);
        }
        i++;
        cout << endl << "Complete!" << endl << endl;
    } //while
    fout.close();

    Result_Output_TXT->close();

    return true;
}