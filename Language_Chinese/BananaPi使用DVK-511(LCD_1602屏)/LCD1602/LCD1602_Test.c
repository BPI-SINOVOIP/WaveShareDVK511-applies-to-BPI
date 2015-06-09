#include <stdio.h>
#include <string.h> 
#include <errno.h>

#include<wiringPi.h>
#include<lcd.h>

const int RS = 3;	// 
const int EN = 14;	// 
const int D0 = 4;	// 
const int D1 = 12;	// 
const int D2 = 13;	// 
const int D3 = 6;	// 


int main(){
if (wiringPiSetup() < 0)
{
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
}

int lcdFD;
lcdFD = lcdInit(2, 16, 4, RS, EN, D0, D1, D2, D3, D0, D1, D2, D3);
lcdPosition(lcdFD, 0,0);
lcdPrintf(lcdFD,"Welcome");

lcdPosition(lcdFD, 0,1);
lcdPrintf(lcdFD, "BananaPi");

}
