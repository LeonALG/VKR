# VKR
В рамках существующего программно-аппаратного комплекса стендовых ударных испытаний было необходимо разработать ПО, реализующее сбор, анализ и накопление следующих характеристик испытания:
1. Выделение момента очередного удара установки по изделию;
2. Подсчет количества ударов;
3. Определение его параметров: момент срыва платформы ударной установки, сила удара, окончание колебаний после удара.

Накопленная информация по испытаниям в дальнейшем будет использоваться для анализа оператором или заказчиком испытания, а также собираться статистика по испытаниям.
Программная составляющая программно-аппаратного комплекса будет разработана для работы с ударным стендом ВСТС 450/1000.
Установка предназначена для испытаний продукции на прочность и устойчивость при воздействии механических ударов одиночного и многократного действия в лабораторных и производственных условиях.
Датчик, закрепленный на ударном стенде, регистрирует ускорение и преобразует его в напряжение пропорциональное этому ускорению, после чего подает напряжение с определенной частотой на модуль АЦП L-Card E14-440. 
Модуль АЦП E14-440, соединѐнный по USB с компьютером, передает на компьютер массив данных. 
Данные представляют собой знаковые двухбайтовые целые числа. Стоит заметить, что существует задержка, равная 0.5 секунды, которая никак не влияет на сбор и анализ данных.
Для преобразования этих данных в физические единицы, а именно в виде ускорения в каждый момент времени (м/с2), необходимо воспользоваться формулой.
За долгие годы работы, на предприятии разработали собственную математическую модель взаимодействия датчиков с компьютером. 
Эта формула описывает линейную зависимость преобразования кодов в физические единицы. 
Она связывает физическую величину, которую регистрирует датчик с численным значением физической величины, которой оперируют в компьютере. 
С помощью phys1, phys2, code1, code2 происходит преобразование кода АЦП, полученного с АЦП, в физическую величину, зарегистрированную датчиком. 
Эти коэффициенты уникальны для каждого датчика и для каждого условия испытаний.
Для работы с проектом необходимо подсоединить АЦП Е14-440 к компьютеру по USB, а также соединить его с датчиком, который будет регистрировать ускорение.
Далее необходимо установить драйвер для работы с АЦП Е14-440 (lcomp).  
Для начала работы с программой, необходимо запустить файл «Client.exe» через консоль с параметрами "Client.exe 0 e440".
После запуска программы, необходимо проверить корректность параметров.
Далее начать сбор данных, нажатием клавиши «1». Остановка сбора производится путем нажатия клавиши «2». Клавиша «0» отвечает за выход из программы.
После сбора данных файлы в форматах «.dat» и «.txt» с результатами испытания сохраняются на стационарный компьютер.
В файле «Result.txt» содержится информация о результатах тестирования, а именно: количество ударов, моменты очередного удара установки по изделию, параметры удара.
В файле «data.dat» содержатся значения ускорений, полученных с АЦП Е14-440 на протяжении всего испытания.
Алгоритм анализа и преобразования данных, полученных с АЦП Е14-440, можно посмотреть здесь: https://github.com/LeonALG/Convert 

Программа для графического отображения данных, полученных в результате испытаний находится здесь: https://github.com/LeonALG/VKR_Graph
