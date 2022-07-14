#include <stdio.h>
#include <stdint.h>

//Пора обновить состояние пина
uint8_t flacDOut300Hzupdate = 0;
uint8_t flacDOut1KHzupdate = 0;
uint8_t flacDOut10KHzupdate = 0;


//Вспомогательные счетчики для других пинов
uint8_t counter1KHz = 0;
uint8_t counter300Hz = 0;
uint8_t extracounter300Hz = 0;

uint8_t flacExtraTimertickFor300HzUpdateNeeded = 0;

//8 бит, значение в попугаях от 0 до 255
uint8_t ADCvalueREG = 0;
uint8_t val[32];
uint16_t bufSum = 0;
//итоговое значение
uint8_t res = 0;
//Счетчик для таймера
uint16_t ADCcounter = 0;
//100 Гц
const uint16_t ADCgoalTicks = 10001;
//Счетчик числа измерений
uint8_t ADCcounterVal = 0;
//Пора считать значение
uint8_t flacADCupdate = 0;



struct __attribute__((__packed__)) Port
{
    uint8_t B0: 1;
    uint8_t B1: 1;
    uint8_t B2: 1;
    uint8_t reserv: 5;

}myPORT;

//300 Гц    = 3333 мкс
//1000 Гц   = 1000 мкс
//10000 Гц  = 100 мкс
//3333/100 = 33 (каждые 99 добавляем 1, т.е. после каждого 3 раза)
//1000/100 = 10
//100/100 = 1

//Число тиков счетчиков
const uint8_t goalTicks1KHz = 11;
const uint8_t goalTicks300Hz = 34;


//Прерывание по таймеру каждые 100 мкс
void TIMER1_IRQ()
{
    flacDOut10KHzupdate = 1;

    counter1KHz++;
    counter300Hz++;
    ADCcounter++;
    //DOUT
    if (counter1KHz == goalTicks1KHz)
    {
        counter1KHz = 0;
        flacDOut1KHzupdate = 1;
    }

    if (counter300Hz == goalTicks300Hz)
    {
        //Если 100 тик
        if (flacExtraTimertickFor300HzUpdateNeeded)
        {
            flacExtraTimertickFor300HzUpdateNeeded = 0;
            flacDOut300Hzupdate = 1;
        }
        counter300Hz = 0;
        extracounter300Hz++;
        //Каждые 99 тиков добавляем еще один тик для точности
        if(counter300Hz == 3)
        {
            flacExtraTimertickFor300HzUpdateNeeded = 1;
        }
        else
        {
            flacDOut300Hzupdate = 1;
        }
    }

    //ADC
    if(ADCcounter == ADCgoalTicks)
    {
        ADCcounter = 0;
        flacADCupdate = 1;
        //запуск преобразования
    }
}

int main()
{
    //Инициализация системного таймера с прерыванием на 100 мкс
    //Порта B. Цифровой выход, высокая скорость
    //Инициализация АЦП (опорный сигнал), установка измерений на 100 Гц, по окончанию преобразования запись результа в ADCvalueREG. Запуск преобразования в ручном режиме
    //Обнуление буфера для АЦП
    for (int i = 0; i < 32; i++)
    {
        val[i] = 0;
    }

    while (1)
    {
        if (flacDOut300Hzupdate)
        {
            myPORT.B0 ^= 1;
        }
        if (flacDOut1KHzupdate)
        {
            myPORT.B1 ^= 1;
        }
        if(flacDOut10KHzupdate)
        {
            myPORT.B2 ^= 1;
        }
        if (flacADCupdate)
        {
            val[ADCcounterVal] = ADCvalueREG;
            ADCcounterVal++;
            //Усредняем
            if (ADCcounterVal == 32)
            {
                for (int i = 0; i< 32; i++)
                {
                    bufSum += val[i];
                }
                //Итоговое значение
                res = (uint8_t)(bufSum/32);
                ADCcounterVal = 0;
            }
        }
    }
}
