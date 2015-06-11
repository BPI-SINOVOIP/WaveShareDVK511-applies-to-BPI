// key.c
//
// Example program for wiringPi library
//
// After installing wiringPi, you can build this
// with something like:
// make or gcc -o key key.c -lwiringPi
// sudo ./led
/*
             define from bcm2835.h                       define from Board DVK511
                 3.3V | | 5V               ->                 3.3V | | 5V
    RPI_V2_GPIO_P1_03 | | 5V               ->                  SDA | | 5V 
    RPI_V2_GPIO_P1_05 | | GND              ->                  SCL | | GND
       RPI_GPIO_P1_07 | | RPI_GPIO_P1_08   ->                  IO7 | | TX
                  GND | | RPI_GPIO_P1_10   ->                  GND | | RX
       RPI_GPIO_P1_11 | | RPI_GPIO_P1_12   ->                  IO0 | | IO1
    RPI_V2_GPIO_P1_13 | | GND              ->                  IO2 | | GND
       RPI_GPIO_P1_15 | | RPI_GPIO_P1_16   ->                  IO3 | | IO4
                  VCC | | RPI_GPIO_P1_18   ->                  VCC | | IO5
       RPI_GPIO_P1_19 | | GND              ->                 MOSI | | GND
       RPI_GPIO_P1_21 | | RPI_GPIO_P1_22   ->                 MISO | | IO6
       RPI_GPIO_P1_23 | | RPI_GPIO_P1_24   ->                  SCK | | CE0
                  GND | | RPI_GPIO_P1_26   ->                  GND | | CE1

::if your raspberry Pi is version 1 or rev 1 or rev A
RPI_V2_GPIO_P1_03->RPI_GPIO_P1_03
RPI_V2_GPIO_P1_05->RPI_GPIO_P1_05
RPI_V2_GPIO_P1_13->RPI_GPIO_P1_13
::
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wiringPi.h>

#define KEY0 17 //wiringPi gpio0
#define KEY1 18 //wiringPi gpio1
#define KEY2 27 //wiringPi gpio2
#define KEY3 22 //wiringPi gpio3
#define KEY4 23 //wiringPi gpio4
#define KEY5 24 //wiringPi gpio5
#define KEY6 25 //wiringPi gpio6
#define KEY7 4  //wiringPi gpio7

char KEY[]={KEY0,KEY1,KEY2,KEY3,KEY4,KEY5,KEY6,KEY7};
int main(int argc, char **argv)
{
    int value,i;
    int key_event[8];
    char gpio_cmd[80];

    if (wiringPiSetup() < 0)
    {
        fprintf (stderr, "Unable to open device: %s\n", strerror (errno)) ;
        return 1 ;
    }

    for(i=0; i<8; i++)
    {
        //KEY initial
        pinMode(KEY[i], INPUT);
        sprintf(gpio_cmd,"echo %d > /sys/class/gpio/export", KEY[i]);
        system(gpio_cmd);
        sprintf(gpio_cmd,"echo in > /sys/class/gpio/gpio%d/direction", KEY[i]);
        system(gpio_cmd);
        sprintf(gpio_cmd,"echo up > /sys/class/gpio/gpio%d/pull", KEY[i]);
        system(gpio_cmd);
    }

    while (1)
    {
        for (i=0; i<8; i++)
        {
            key_event[i] = digitalRead(i);
        }
        printf("KeyPress(%d;%d;%d;%d;%d;%d;%d;%d)\r", 
                key_event[0], key_event[1], key_event[2], key_event[3],
                key_event[4], key_event[5], key_event[6], key_event[7]);
	
        delay(3);
/*
        for(i=0;i<8;i++)
        {
            if(digitalRead(i)==0)        
            {
                printf("press the key: %d\n", i);
                delay(250);
            }
        }
*/
    }

    return 0;
}
