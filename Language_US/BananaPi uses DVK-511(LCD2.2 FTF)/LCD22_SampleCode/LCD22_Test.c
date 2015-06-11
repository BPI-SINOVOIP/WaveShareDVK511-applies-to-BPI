/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "asciihex8X16.h"

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long int

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
//static uint32_t speed = 500000;
//static uint32_t speed = 5000000;
static uint32_t speed = 20000000;
static uint16_t delay_time;
static int f_initlcd;
static int lcdtest;
static int x;
static int y;
static int fcolor=4; //BLACK
static int bgcolor=3; //YELLOW
static char *show;
static char *picfile;
int fd;
void *fbtxbuf;
void *fbrxbuf;

static void spi_rw(u8 data)
{
	int ret;
	uint8_t tx[1] = {
		0x9f,
	};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay_time,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	tx[0] = data;
//	ret = write(fd, &data, 1);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
}

static void spi_rw2(u16 data)
{
	int ret;
	uint8_t tx[2] = {
		0x9f,
	};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay_time,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	tx[0] = (u8)(data>>8);
	tx[1] = (u8)(data);
//	ret = write(fd, &data, 1);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
}

static void spi_rw240(u16 data)
{
	int ret;
	uint8_t tx[480] = {
		0x9f,
	};
	int i;
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay_time,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	tx[0] = (u8)(data>>8);
	tx[1] = (u8)(data);
	for(i=1;i<240;i++) {
		tx[i*2] = tx[0];
		tx[i*2+1] = tx[1];
	}
//	ret = write(fd, &data, 1);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
}

static void spi_rwfb(u16 data)
{
	int ret;
	unsigned long tx;
	unsigned long rx;
	int i;
	u8 b1,b2;
	u8 *buf;
	u8 *ptr;
	int byteleft;
	struct spi_ioc_transfer tr = {
#ifdef false
		.len = 240*320*2, //ERROR: can't send spi message: Message too long
#else
		.len = 4096, // OK
#endif
		.delay_usecs = delay_time,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
//	printf("fbtxbuf=[%x]\n",fbtxbuf);
//	printf("fbrxbuf=[%x]\n",fbrxbuf);
	tr.tx_buf = (unsigned long)fbtxbuf;
	tr.rx_buf = (unsigned long)fbrxbuf;
	buf = (u8 *)fbtxbuf;
	b1 = (u8)(data>>8);
	b2 = (u8)(data);
	for(i=1;i<240*320;i++) {
		buf[i*2] = b1;
		buf[i*2+1] = b2;
	}
	byteleft=240*320*2;
	ptr=buf;
#if 0 
//	ret = write(fd, buf, 240*320*2); //ERROR: too big
//	ret = write(fd, buf, 4096); //OK
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
#endif
	while(byteleft>=4096) {
		ret = write(fd, ptr, 4096);
		if (ret < 1)
			pabort("can't send spi message");
		ptr += 4096;
		byteleft -= 4096;
	}
	ret = write(fd, ptr, byteleft);
	if (ret < 1)
		pabort("can't send spi message");
	
}

static void spi_updatefb(u8 *inbuf)
{
	int ret;
	u8 *buf;
	u8 *ptr;
	int byteleft;
	buf = inbuf;
	byteleft=240*320*2;
	ptr=buf;
	while(byteleft>=4096) {
		ret = write(fd, ptr, 4096);
		if (ret < 1)
			pabort("can't send spi message");
		ptr += 4096;
		byteleft -= 4096;
	}
	ret = write(fd, ptr, byteleft);
	if (ret < 1)
		pabort("can't send spi message");
	
}

int spi_init()
{
	int ret = 0;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return ret;
}


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


#define reset_clr()	system("echo 0 > /sys/class/gpio/gpio25/value");//RST
#define reset_set()	system("echo 1 > /sys/class/gpio/gpio25/value");//RST

#define en_lcd()	system("echo 0 > /sys/class/gpio/gpio8/value");//CS
#define dis_lcd()	system("echo 1 > /sys/class/gpio/gpio8/value");//CS

#define en_lcd_index()	system("echo 0 > /sys/class/gpio/gpio7/value");//RS
#define en_lcd_data()	system("echo 1 > /sys/class/gpio/gpio7/value");//RS



#define COLOR_YELLOW 		0xFFE0
#define COLOR_BLACK 		0x0000
#define COLOR_WHITE 		0xFFFF
#define COLOR_INIT 		COLOR_YELLOW



#define	D	printf("BananaPi:__%d__(%s)\n",__LINE__,__FUNCTION__);

const u16 colorfol[]={0xf800,0x07e0,0x001f,0xffe0,0x0000,0xffff,0x07ff,0xf81f};

void init_lcd_spi_gpio(void)
{    
	system("echo 25 > /sys/class/gpio/export");
	system("echo 8 > /sys/class/gpio/export");
	system("echo 7 > /sys/class/gpio/export");
	system("echo out > /sys/class/gpio/gpio25/direction");
	system("echo out > /sys/class/gpio/gpio8/direction");
	system("echo out > /sys/class/gpio/gpio7/direction");
}

void post_data(u16 data)
{
	spi_rw2(data);
}

void lcd_rst(void) 
{
D	reset_clr();
	usleep(100);
	reset_set();
	usleep(100);
}

void post_cmd(u16 index, u16 cmd)
{
	en_lcd_index();
	post_data(index);
	en_lcd_data();
	post_data(cmd);
}

void init_lcd(void)
{

D	en_lcd();
	lcd_rst();

	post_cmd( 0x000, 0x0001 ); /* oschilliation start */
	usleep( 10 );
	/* Power settings */  	
	post_cmd( 0x100, 0x0000 ); /*power supply setup*/	
	post_cmd( 0x101, 0x0000 ); 
	post_cmd( 0x102, 0x3110 ); 
	post_cmd( 0x103, 0xe200 ); 
	post_cmd( 0x110, 0x009d ); 
	post_cmd( 0x111, 0x0022 ); 
	post_cmd( 0x100, 0x0120 ); 
	usleep( 20 );

	post_cmd( 0x100, 0x3120 );
	usleep( 80 );
	/* Display control */   
	post_cmd( 0x001, 0x0100 );
	post_cmd( 0x002, 0x0000 );
	post_cmd( 0x003, 0x1230 );
	post_cmd( 0x006, 0x0000 );
	post_cmd( 0x007, 0x0101 );
	post_cmd( 0x008, 0x0808 );
	post_cmd( 0x009, 0x0000 );
	post_cmd( 0x00b, 0x0000 );
	post_cmd( 0x00c, 0x0000 );
	post_cmd( 0x00d, 0x0018 );
	/* LTPS control settings */   
	post_cmd( 0x012, 0x0000 );
	post_cmd( 0x013, 0x0000 );
	post_cmd( 0x018, 0x0000 );
	post_cmd( 0x019, 0x0000 );

	post_cmd( 0x203, 0x0000 );
	post_cmd( 0x204, 0x0000 );

	post_cmd( 0x210, 0x0000 );
	post_cmd( 0x211, 0x00ef );
	post_cmd( 0x212, 0x0000 );
	post_cmd( 0x213, 0x013f );
	post_cmd( 0x214, 0x0000 );
	post_cmd( 0x215, 0x0000 );
	post_cmd( 0x216, 0x0000 );
	post_cmd( 0x217, 0x0000 );

	// Gray scale settings
	post_cmd( 0x300, 0x5343);
	post_cmd( 0x301, 0x1021);
	post_cmd( 0x302, 0x0003);
	post_cmd( 0x303, 0x0011);
	post_cmd( 0x304, 0x050a);
	post_cmd( 0x305, 0x4342);
	post_cmd( 0x306, 0x1100);
	post_cmd( 0x307, 0x0003);
	post_cmd( 0x308, 0x1201);
	post_cmd( 0x309, 0x050a);

	/* RAM access settings */ 
	post_cmd( 0x400, 0x4027 );
	post_cmd( 0x401, 0x0000 );
	post_cmd( 0x402, 0x0000 );	/* First screen drive position (1) */   	
	post_cmd( 0x403, 0x013f );	/* First screen drive position (2) */   	
	post_cmd( 0x404, 0x0000 );

	post_cmd( 0x200, 0x0000 );
	post_cmd( 0x201, 0x0000 );
	
	post_cmd( 0x100, 0x7120 );
	post_cmd( 0x007, 0x0103 );
	usleep( 10 );
	post_cmd( 0x007, 0x0113 );

	dis_lcd();
}

void LCD_home()
{
	u16  temp,num;
	u8 n,i;
	en_lcd();	
	post_cmd(0x210,0x00);
	post_cmd(0x212,0x0000);
	post_cmd(0x211,0xEF);
	post_cmd(0x213,0x013F);
//	
	post_cmd(0x200,0x0000);
	post_cmd(0x201,0x0000);
	en_lcd_index();
	post_data(0x202);
	en_lcd_data();
	dis_lcd();
}

void LCD_colorbar()
{
	u16  temp,num;
	u8 n,i;
	LCD_home();
	en_lcd();	
	for(n=0;n<8;n++) {
		temp=colorfol[n];
		for(num=0;num<40;num++) {
			spi_rw240(temp);
		}
	}
	dis_lcd();
}

void LCD_fill(u16 color)
{
	int i;
	LCD_home();
	en_lcd();	
	spi_rwfb(color);
	dis_lcd();
}

void LCD_fill2(u16 color)
{
	int i;
	LCD_home();
	en_lcd();	
	for(i=0;i<320;i++) {
		spi_rw240(color);
	}
	dis_lcd();
}

void LCD_test(void)
{
	u16 color;
	int i;
	LCD_colorbar();
	for(i=0;i<8;i++) {
		color=colorfol[i];
		LCD_fill(color);
	}
}


void DisplayChar(u8 casc,int x,int y)
{
	u8 i,j,b;
	const u8 *p;
	u8 *fbuf = (u8 *)fbtxbuf;
	u8 *fp;
	int offset;

	offset = (x + y*240)*2;
	fp = fbuf + offset;
//printf("x(%d) y(%d)\n",x,y);
//printf("fbtxbuf[%x] fbuf[%x] fp[%x] offset(%d)\n",fbtxbuf,fbuf,fp,offset);
	
	p=ascii;
	p+=casc*16;
	for(j=0;j<16;j++) {
		b=*(p+j);
		for(i=0;i<8;i++) {
			if(b&0x80) {
				*fp++=colorfol[fcolor]>>8;
				*fp++=colorfol[fcolor];
			}
			else {
				*fp++=colorfol[bgcolor]>>8;
				*fp++=colorfol[bgcolor];
			}
			b=b<<1;
		}	
		fp = fbuf + offset + j*240*2;
	}
}

void DisplayChar2(u8 casc,u8 postion_x,u8 postion_y)
{
	u8 i,j,b;
	const u8 *p;
	
	en_lcd();
	post_cmd(0x210,postion_x*8); 	//x start point
	post_cmd(0x212,postion_y*16); 	//y start point
	post_cmd(0x211,postion_x*8+7);	//x end point
	post_cmd(0x213,postion_y*16+15);	//y end point
	post_cmd(0x200,postion_x*8);	
	post_cmd(0x201,postion_y*16);
	
	en_lcd_index();
	post_data(0x202);
	en_lcd_data();
	p=ascii;
	p+=casc*16;
	for(j=0;j<16;j++)
	{
		b=*(p+j);
		for(i=0;i<8;i++)
		{
			if(b&0x80)
			{
//				post_data(COLOR_BLACK);
				post_data(colorfol[fcolor]);
			}
			else
			{
//				post_data(COLOR_YELLOW);
				post_data(colorfol[bgcolor]);
			}
			b=b<<1;
			
		}	
	}
	dis_lcd();
}

void DisplayString(u8 *s,u8 x,u8 y,u8 Reverse)
{
	u8 a[10],i;
	while (*s) {
		DisplayChar(*s,x,y);
		x+=8;
		if(x>=240) {
			x=0;
			y += 20;
			if(y>=320)
				y=0;
		}
		s++;
	}
}

void DisplayString2(u8 *s,u8 x,u8 y,u8 Reverse)
{
	u8 a[10],i;
	while (*s) 
	{ 
		{DisplayChar2(*s,x,y);}
		if(++x>=30)
		{
			x=0;
			if(++y>=20)
			{
			  y=0;
			}
		}
		s++;
    }
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n"
	     "  -x --x        x for show\n"
	     "  -y --y        y for show\n"
	     "  -f --fcolor   font color\n"
	     "  -c --bgcolor  bg color\n"
	     "  -S --show     show LCD strings\n"
	     "  -p --picfile  show pic file\n"
	     "  -T --lcdtest  Lcd Test\n"
	     "  -I --lcdinit  Lcd Init\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ "lcdtest", 0, 0, 'T' },
			{ "lcdinit", 0, 0, 'I' },
			{ "x",       1, 0, 'x' },
			{ "y",       1, 0, 'y' },
			{ "f",       1, 0, 'f' },
			{ "c",       1, 0, 'c' },
			{ "show",    1, 0, 'S' },
			{ "pic",     1, 0, 'p' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:x:y:f:c:S:p:lHOLC3NRTI", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay_time = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		case 'T':
			lcdtest++;
			break;
		case 'I':
			f_initlcd++;
			break;
		case 'x':
			x = atoi(optarg);
			break;
		case 'y':
			y = atoi(optarg);
			break;
		case 'f':
			fcolor = atoi(optarg);
			if(fcolor<0 || fcolor>7)
				fcolor=4;
			break;
		case 'c':
			bgcolor = atoi(optarg);
			if(bgcolor<0 || bgcolor>7)
				bgcolor=3;
			break;
		case 'S':
			show = optarg;
			break;
		case 'p':
			picfile = optarg;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned readbuf_size;
	int picfd;

	readbuf_size = 240 * 320 * 2;
	fbtxbuf = malloc(readbuf_size);
	if(fbtxbuf==NULL)
		return 1;
	fbrxbuf = malloc(readbuf_size);
	if(fbrxbuf==NULL)
		return 1;

	parse_opts(argc, argv);

D	spi_init();
	if(f_initlcd) {
D		init_lcd_spi_gpio();
		init_lcd();
	}
	if(lcdtest) {
		LCD_test();
		DisplayString("Banana Pi",20,20,0);
		DisplayString("DVK511 LCD22(SPI)",20,40,0);
		LCD_home();
		en_lcd();	
		spi_updatefb((u8 *)fbtxbuf);
		dis_lcd();	
	}
	if(show) {
		printf("BananaPi: (%s)\n",show);
		DisplayString2(show,x,y,0);
	}
	if(picfile) {
		picfd = open(picfile, O_RDWR);
		if (picfd < 0)
			pabort("can't open picfile");
		read(picfd, fbrxbuf, 240*320*2);
		LCD_home();
		en_lcd();	
		spi_updatefb((u8 *)fbrxbuf);
		dis_lcd();	
	}
	return 0;
}
