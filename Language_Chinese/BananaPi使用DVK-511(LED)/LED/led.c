// blink.c
//
// Example program for wiringPi library
// Blinks a pin on an off every 0.5 secs
//
// After installing wiringPi, you can build this 
// with something like:
// make or gcc -o led led.c -lwiringPi
// sudo ./led
/*
 +----------+-Rev3-+------+--------+------+-------+
| wiringPi | GPIO | Phys | Name   | Mode | Value |
+----------+------+------+--------+------+-------+
|      0   |  17  |  11  | GPIO 0 | ALT4 | Low   |
|      1   |  18  |  12  | GPIO 1 | IN   | Low   |
|      2   |  27  |  13  | GPIO 2 | ALT4 | Low   |
|      3   |  22  |  15  | GPIO 3 | ALT4 | Low   |
|      4   |  23  |  16  | GPIO 4 | IN   | Low   |
|      5   |  24  |  18  | GPIO 5 | IN   | Low   |
|      6   |  25  |  22  | GPIO 6 | ALT4 | Low   |
|      7   |   4  |   7  | GPIO 7 | IN   | Low   |
|      8   |   2  |   3  | SDA    | ALT5 | Low   |
|      9   |   3  |   5  | SCL    | ALT5 | Low   |
|     10   |   8  |  24  | CE0    | IN   | Low   |
|     11   |   7  |  26  | CE1    | IN   | Low   |
|     12   |  10  |  19  | MOSI   | IN   | Low   |
|     13   |   9  |  21  | MISO   | IN   | Low   |
|     14   |  11  |  23  | SCLK   | IN   | Low   |
|     15   |  14  |   8  | TxD    | ALT0 | Low   |
|     16   |  15  |  10  | RxD    | ALT0 | Low   |
|     17   |  28  |   3  | GPIO 8 | IN   | Low   |
|     18   |  29  |   4  | GPIO 9 | ALT4 | Low   |
|     19   |  30  |   5  | GPIO10 | OUT  | High  |
|     20   |  31  |   6  | GPIO11 | ALT4 | Low   |
+----------+------+------+--------+------+-------+

*/
#include <stdio.h>
#include <errno.h>
#include <wiringPi.h>


#define LED1 0
#define LED2 1
#define LED3 2
#define LED4 3
#define LED5 4
#define LED6 5
#define LED7 6
#define LED0 7

char LED[8]={LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED0};
int main(int argc, char **argv)
{
    if (wiringPiSetup() < 0)
    {
        fprintf (stderr, "Unable to open device: %s\n", strerror (errno)) ;
        return 1 ;
    }
    // Blink
    char i;

    for(i=0;i<8;i++){
       pinMode(LED[i], OUTPUT);
    }

    while (1)
    {
	for(i=0;i<8;i++)
	{	
                digitalWrite(LED[i],HIGH);
       	        delay(500);
               	digitalWrite(LED[i],LOW);
                delay(500);
	}
    }


    return 0;
}
