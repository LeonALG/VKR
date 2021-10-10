//==============================================================================
// Гречишкин А.А., 17.01.2021 https://www.youtube.com/watch?v=ca1fwd1LEDw&feature=youtu.be
//==============================================================================

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>

#include <windows.h>
#include "Convert.h"

using namespace std;

// Константы именно для данного файла.
const double phys1 = 0.0;
const double phys2 = 36.03;
const double code1 = -90.6653;
const double code2 = -15552.47;
const double k = (phys2 - phys1) / (code2 - code1);

// Файл вывода преобразованного файла для просмотра в лазарус
const char* Result_fileNameOut = "Result.txt"; // Файл с результатами ударов

/*----------------------------------------------------------------------------------*/
/*Характеристики поиска*/
double FuturePoint = 0.08; // Значение будущей точки при поиске ключевой точки (прыжок на)
int Primary_Pit_size = 5; // Начальное значение размера участка ямы 
double Primary_Impact_size = 0.3; // Начальное значение размера участка удар
int Primary_Noise_size = 40; // Начальное значение размера участка шум
int FreeFallSensitivityLimit = 0; // Граница чувствительности свободного падения
//double Multiplier_Comparison_Max_Value_For_Short_fall = 0.6; // Коэф. чтобы понять, что попали в небольшую пропасть (обрыв)
double Multiplier_Comparison_Max_Value_For_Long_fall = 0.85; // Коэф. чтобы понять, что попали в большую пропасть (обрыв)
double Noise_min_value = -0.7; // Минимальное значение ускорения, чтобы определить шум
double Noise_max_value = 1.0;  // Максимальное значение ускорения, чтобы определить шум

int Pit_size = 1000; // Максимальное количество подряд идущих точек для определения участка как яма
int Impact_size = 30; // Максимальное количество подряд идущих точек для определения участка как удар
int Noise_size = 40; // Максимальное количество подряд идущих точек для определения участка как шум
/*--------------------*/

bool flag_negative_area = false;  // нахождение попадания в отрицательную область (но не означает, что мы нашли яму)
bool FreeFallMoment = false; //наличие области свободного падения (далее "яма")

bool flag_positive_area = false; // нахождение попадания в положительную область (но не означает, что мы нашли удар)
bool Impact_In_Progress = false; // флаг на то, что удар находится в процессе (если так можно сказать)

bool flag_area_Noise = false; // нахождение окончания колебаний (попадаем в шум)
bool flag_Noise_moment = false; // нахождение попадания в область шума (но не означает, что мы нашли шум)

ULONG j; // счетчик
int StartFreeFall = 0; // точка начала свободного падения
int CountFreeFall = 0; // количество вхождений в отрицательную область
int StartNoise = 0; // точка начала шума
int CountNoise = 0; // количество вхождений в область шума

int Count_Moment_Of_Impact = 0; // количество точек после момента удара
int Starting_Point_Impact = 0; // точка удара
int Number_Output = 0; // номер вывода

double Max_Value = 0.0; // Потенциальная ключевая точка
int Max_Value_coord = 0; // Индекс максимальной точки в массиве
bool Max_Value_flag = false; // Флаг надобности поиска ключевой точки
bool Max_Find_Complete = false; // Флаг, что найдена ключевая точка
double EditFrequencyValue = 20; // Ввод пользователем частоты, на которой был записан файл

const int Value_after_Сomma = 12; //Количество значков после запятой
ofstream* Result_Output_TXT = NULL; // Файл вывода результатов

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
    // Если ускорение меньше границы чувствительности
    if (MainPoint[j] < FreeFallSensitivityLimit) {
        // Если не входили в потенциальную яму и не находили яму, тогда 
        if ((flag_negative_area == false) and (FreeFallMoment == false)) {
            // Запоминаем возможную точку начала ямы
            // Ставим флаг на то, что находимся в отрицательной области
            StartFreeFall = j;
            half = i;
            flag_negative_area = true;
            CountFreeFall = 0;
        }
    }
    // Если ускорение больше границы чувствительности свободного падения
    // и Если предположительно были в яме
    else if (flag_negative_area == true)
    {
        // то уже не в яме  
        flag_negative_area = false;
        CountFreeFall = 0;
    };

    // Если нашли отрицательную область, тогда считаем количество точек для определения ямы
    if (flag_negative_area == true)
        CountFreeFall++;

    // Если количество подряд идущих точек в отрицательной области больше 1000 и
    // если мы еще не входили в яму
    if ((CountFreeFall > Pit_size) and (FreeFallMoment == false)) {
        // Отрицательной области нет
        // Нашли яму
        // Обнуляем количество подряд идущих точек
        flag_negative_area = false;
        FreeFallMoment = true;
        Number_Output++;
        CountFreeFall = 0;
        cout << "\n[" << Number_Output << "]" << endl;
        *Result_Output_TXT << "[" << Number_Output << "]" << endl;
        cout << "Pit =  " << StartFreeFall + (half * CountPoint) << endl;
        *Result_Output_TXT << "Pit =  " << StartFreeFall + (half * CountPoint) << endl;
        // Если уже был шум, то шума нет
        if (flag_Noise_moment == true)
            flag_Noise_moment = false;
        return true;
    };
    return false;
}

bool ConvertADC::FindImpact(int j, int i, int CountPoint)
{
    // Если ускорение больше границы чувствительности
    if (MainPoint[j] > FreeFallSensitivityLimit)
    {
        // Если не было вхождения в положительную область
        if (flag_positive_area == false)
        {
            // Мы нашли положительную область, запоминаем точку, где она нашлась
            flag_positive_area = true;
            Count_Moment_Of_Impact = 0;
            Starting_Point_Impact = j;
            half = i;
        }
    }
    // Если положительная область уже была
    else if (flag_positive_area == true)
    {
        // Обнуляем ее
        flag_positive_area = false;
        Count_Moment_Of_Impact = 0;
    }

    // Если мы нашли положительную область, считаем количество точек в ней, для определения удара 
    if (flag_positive_area == true)
        Count_Moment_Of_Impact++;

    // Если количество ударов после положительной области больше той, которая необходима для определения
    // точки как удар и удара еще не было
    if ((Count_Moment_Of_Impact > Impact_size) and (Impact_In_Progress == false)) {
        // Обнуляем положительную область
        flag_positive_area = false;
        // Происходит удар
        Impact_In_Progress = true;
        // Момент свободного падения нам не нужен = false
        FreeFallMoment = false;
        // Обнуляем количество подряд идущих точек
        Count_Moment_Of_Impact = 0;
        // Присваиваем потенциальной ключевой точке значение точки начала удара (y)
        Max_Value = MainPoint[j - Impact_size];
        // Присваиваем потенциальной ключевой точке значение точки начала удара (x)
        Max_Value_coord = j - Impact_size;
        // Вывод
        cout << "Point of impact = " << Starting_Point_Impact + (half * CountPoint) << endl;
        *Result_Output_TXT << "Point of impact = " << Starting_Point_Impact + (half * CountPoint) << endl;
        return true;
    };
    return false;
}

bool ConvertADC::FindMaxPoint(int j, int i, int CountPoint)
{
    // Если дальше в точке будет обрыв и ускорения больше 1.5
    if ((Max_Value * Multiplier_Comparison_Max_Value_For_Long_fall > MainPoint[j + static_cast<int>(round(FuturePoint * EditFrequencyValue))]) &&
        (MainPoint[j] > 1.5))
    {
        // Пока не спуск проходим на координату вперед
        while (MainPoint[Max_Value_coord] <= MainPoint[Max_Value_coord + 1])
            Max_Value_coord++;
        Max_Find_Complete = true;
        // Вывод
        cout << "Point of maximum = " << Max_Value_coord + (half * CountPoint) << endl;
        cout << "Value of maximum = " << setprecision(Value_after_Сomma) << MainPoint[Max_Value_coord] << endl;
        *Result_Output_TXT << "Point of maximum = " << Max_Value_coord + (half * CountPoint) << endl;
        *Result_Output_TXT << "Value of maximum = " << setprecision(Value_after_Сomma) << MainPoint[Max_Value_coord] << endl;
        Impact_In_Progress = false;
        return true;
    }
    // Если текущее значение больше текущего максимума, то новые значения
    else if (MainPoint[j] > Max_Value)
    {
        Max_Value = MainPoint[j];
        Max_Value_coord = j;
    }
    return false;
}

bool ConvertADC::FindNoise(int j, int i, int CountPoint)
{
    // Если текущее ускорение находится в промежутке определения шума
    if ((MainPoint[j] > Noise_min_value) and (MainPoint[j] < Noise_max_value)) {
        // Если шума еще не было и еще не заходили в область шума
        if (flag_area_Noise == false) {
            // В область шума зашли и запомнили координату начала шума
            flag_area_Noise = true;
            StartNoise = j;
            half = i;
            CountNoise = 0;
        };
    }
    // Если текущее ускорение находится вне промежутка
    // и Если была область шума
    else if (flag_area_Noise == true)
    {
        // Обнуляем
        CountNoise = 0;
        flag_area_Noise = false;
    }

    // Если была найдена область шума, считаем количество точек в ней, для определения шума
    if (flag_area_Noise == true)
        CountNoise++;

    // Если количество подряд идущих точек больше того, которое необходимо 
    // для определения шума и шум закончился
    if ((CountNoise > Noise_size) and (flag_Noise_moment == false)) {
        // Ставим флаг на нахождение ключевой точки false
        Max_Find_Complete = false;
        // Флаг на нахождение шума true
        flag_Noise_moment = true;
        // Больше не ищем шум`
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
                //Поиск ямы
                FindPit(j, i, size);
            else if (FreeFallMoment == true)
                //Поиск удара
                FindImpact(j, i, size);
            else if (Impact_In_Progress == true)
                //Поиск ключевой точки
                FindMaxPoint(j, i, size);
            else if (Max_Find_Complete == true)
                //Поиск шума
                FindNoise(j, i, size);
        }
        i++;
        cout << endl << "Complete!" << endl << endl;
    } //while
    fout.close();

    Result_Output_TXT->close();

    return true;
}