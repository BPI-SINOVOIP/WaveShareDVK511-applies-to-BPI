/*
 * serialTest.c:
 *	Very simple program to test the serial port. Expects
 *	the port to be looped back to itself
 *
 */

/*
             define from  wiringPi.h                     define from Board DVK511
                 3.3V | | 5V               ->                 3.3V | | 5V
                8/SDA | | 5V               ->                  SDA | | 5V
                9/SCL | | GND              ->                  SCL | | GND
                    7 | | 14/TX            ->                  IO7 | | TX
                  GND | | 15/RX            ->                  GND | | RX
                    0 | | 18               ->                  IO0 | | IO1
                    2 | | GND              ->                  IO2 | | GND
                    3 | | 23               ->                  IO3 | | IO4
                  VCC | | 24               ->                  VCC | | IO5
              MOSI/12 | | GND              ->                 MOSI | | GND
              MISO/13 | | 25               ->                 MISO | | IO6
               SCK/14 | | 8/CE0            ->                  SCK | | CE0
                  GND | | 9/CE19           ->                  GND | | CE1
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int main (int argc, char **argv)
{
  int fd ;
  char count[9]= "\n welcom\n";
  int i_get = 0;
  char devname_head[10] = "/dev/";
  char dev_name[20];

  if(argc < 2)
  {
    printf("Please input './test_uart ttySx'\n");
    return 1;
  }
  else
  {
    strcpy(dev_name, devname_head);
    strcat(dev_name, argv[1]);
  }

  if ((fd = serialOpen (dev_name, 115200)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }
  printf("serial test start:\n");
  
  serialPuts(fd,count);  
  serialPuts(fd,"\n ok serial com!\n");
    
while(i_get != 27)
{
   i_get = serialGetchar(fd);
   serialPutchar(fd, i_get);

   //printf("%c", i_get);
   printf("%c\n", i_get);
  
   delay(1);
}
  printf("\n");
  
  serialClose(fd);
  return 0 ;
}
