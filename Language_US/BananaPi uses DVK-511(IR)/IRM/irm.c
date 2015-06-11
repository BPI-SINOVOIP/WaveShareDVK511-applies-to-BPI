// Blinks a pin on an off every 0.5 secs
// 
// After installing wiringPi, you can build this
// with something like:
// make or gcc -o irm irm.c -lwiringPi
// sudo ./irm
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
#include <errno.h>
#include<wiringPi.h>

#define PIN 1 //PWM (pin12)
int main(void)
{

    if (wiringPiSetup() < 0)
    {
        fprintf (stderr, "Unable to open device: %s\n", strerror (errno)) ;
        return 1 ;
    }

    pinMode(PIN, INPUT);
    pullUpDnControl(PIN, PUD_DOWN);
    printf("irm test start: \n");

    char turn;    
    char n,p,result;
    unsigned long i;
    unsigned long buff[33];
    int val;

    while (1)
    {	
        i++;
        delayMicroseconds(60); //delay 60us
        val = digitalRead(PIN);
        //printf("## i=%d, n=%d, turn=%d val=%d\n", i, n, turn, val);

        if(val == 1)
        {
            turn=1;
            //printf("digitalRead(PIN)==1, turn=1\n");
        }	
	    else
	    {
            if(turn==1)
            {
                if((n>0)&&(n<=33)) 
                    buff[n-1]=i;			
                n++;i=0;

                if(n==34)
                {
                    n = 0;
                    //for(p=0;p<33;p++)
                    //    printf("%d-%d;",p,buff[p]);

                    if(buff[0]>180 && buff[0]<250 && buff[1]<25 && 
                        buff[2]<25 && buff[3]<25 && buff[4]<25 && 
                        buff[5]<25 && buff[6]<25 && buff[7]<25 && 
                        buff[8]<25 && buff[9]>25 && buff[10]>25 && 
                        buff[11]>25 && buff[12]>25 && buff[13]>25) 
                    {
                        for(p=0;p<8;p++)
                        {
                            result>>=1;
                            if(buff[25+p]>25) 
                                result|=0x80;
                        }    	
                        printf("get the irm key code-hex 0x%x \n", result);
                    }
                    else
                    {
                        printf("get the irm key error! \n");
                    }
                    delay(200); //delay 200ms
                }
            }
            turn=0;
        }
    }

    return 0;
}
