/*
 * crpi_gpio.h
 *
 * Raspberry PI C++ GPIO Interface
 * Based off of wiringPi by Gordon Henderson <http://wiringpi.com/>
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "crpi_gpio.h"

#define	PI_GPIO_MASK       (0xFFFFFFC0)
#define BCM2708_PERI_BASE  0x20000000
#define GPIO_BASE          (BCM2708_PERI_BASE + 0x00200000)
#define	GPPUD              37

#define BLOCK_SIZE         (4*1024)

crpi_gpio::crpi_gpio()
{
	fd = -1;
	boardRev = -1;
	gpio = NULL;
}

crpi_gpio::~crpi_gpio()
{
	if (gpio != NULL) {
		munmap(gpio, BLOCK_SIZE);
	}
	if (fd != -1) {
		close(fd);
	}
}

int crpi_gpio::init_gpio()
{
	if (geteuid () != 0)
		return -1;

	if (boardRev == -1)
		boardRev = get_board_rev();
	if (boardRev == -1)
		return -1;

	fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC);
	if (fd == -1)
		return -1;

	gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);

	if ((int32_t)gpio == -1) {
		close(fd);
		fd = -1;
		return -1;
	}
	return boardRev;
}

void crpi_gpio::set_pin_mode(int pin, int mode)
{
	if (boardRev == -1)
		return;

	static uint8_t gpioToGPFSEL[] = {
		0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,
	};

	static uint8_t gpioToShift[] = {
		0,3,6,9,12,15,18,21,24,27,
		0,3,6,9,12,15,18,21,24,27,
		0,3,6,9,12,15,18,21,24,27,
		0,3,6,9,12,15,18,21,24,27,
		0,3,6,9,12,15,18,21,24,27,
	};

	pin = pinToGpio(pin);
	int fSel = gpioToGPFSEL[pin];
	int shift = gpioToShift[pin];

	if (mode == 0)  // Input
		*(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift));
	else if (mode == 1)  // Output
		*(gpio + fSel) = (*(gpio + fSel) & ~(7 << shift)) | (1 << shift);
}

void crpi_gpio::set_pull_mode(int pin, int pud)
{
	if (boardRev == -1)
		return;

	static uint8_t gpioToPUDCLK[] = {
		38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,38,
		39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,
	};

	if ((pin & PI_GPIO_MASK) == 0) {
		pin = pinToGpio(pin);
		*(gpio + GPPUD) = pud & 3;
		delayMicroseconds(5) ;
		*(gpio + gpioToPUDCLK[pin]) = 1 << (pin & 31);
		delayMicroseconds(5) ;
		*(gpio + GPPUD) = 0;
		delayMicroseconds(5) ;
		*(gpio + gpioToPUDCLK[pin]) = 0;
		delayMicroseconds(5) ;
	}
}

int crpi_gpio::read_pin(int pin)
{
	if (boardRev == -1)
		return 0;

	static uint8_t gpioToGPLEV[] = {
		13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
		14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
	};

	if ((pin & PI_GPIO_MASK) == 0) {
		pin = pinToGpio(pin);
		if ((*(gpio + gpioToGPLEV[pin]) & (1 << (pin & 31))) != 0)
			return 1;
	}
	return 0;
}

void crpi_gpio::write_pin(int pin, int value)
{
	if (boardRev == -1)
		return;

	static uint8_t gpioToGPCLR[] = {
		10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
		11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
	};
	static uint8_t gpioToGPSET[] = {
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	};

	if ((pin & PI_GPIO_MASK) == 0) {
		pin = pinToGpio(pin);
		if (value == 0)
			*(gpio + gpioToGPCLR[pin]) = 1 << (pin & 31) ;
		else *(gpio + gpioToGPSET[pin]) = 1 << (pin & 31) ;
	}
}

void crpi_gpio::delayMicroseconds(unsigned int dur)
{
	if (dur == 0)
		return;
	if (dur < 100) {
		struct timeval tNow, tLong, tEnd;

		gettimeofday(&tNow, NULL);
		tLong.tv_sec = dur / 1000000;
		tLong.tv_usec = dur % 1000000;
		timeradd(&tNow, &tLong, &tEnd);

		while (timercmp (&tNow, &tEnd, <))
			gettimeofday(&tNow, NULL);
	} else {
		struct timespec sleeper;
		unsigned int uSecs = dur % 1000000;
		unsigned int wSecs = dur / 1000000;

		sleeper.tv_sec = wSecs;
		sleeper.tv_nsec = (long)(uSecs * 1000L);
		nanosleep(&sleeper, NULL);
	}
}

int crpi_gpio::pinToGpio(int p)
{
	static int pinToGpioR1[64] = {
		17, 18, 21, 22, 23, 24, 25,  4,  0,  1,  8, 7,  10,
		 9, 11, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};

	static int pinToGpioR2[64] = {
		17, 18, 27, 22, 23, 24, 25,  4,  2,  3,  8,  7, 10,
		 9, 11, 14, 15, 28, 29, 30, 31, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};
	if (boardRev == 1)
		return pinToGpioR1[p];
	return pinToGpioR2[p];
}

int crpi_gpio::get_board_rev()
{
	FILE *cpuFd ;
	char buf[128], *c;
	bool b = false;

	cpuFd = fopen("/proc/cpuinfo", "r");
	if (cpuFd == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), cpuFd) != NULL) {
		if (strncmp(buf, "Revision", 8) == 0) {
			b = true;
			break ;
		}
	}

	fclose (cpuFd);

	if (b == false)
		return -1;

	for (c = &buf[strlen(buf)-1]; (*c == '\n') || (*c == '\r'); c--)
		*c = '\0';

	b = false;
	for (c = buf; *c != '\0'; c++) {
		if (isdigit (*c)) {
			b = true;
			break;
		}
	}

	if (b == false)
		return 1;

	c = c+strlen(c)-1;

	if ((*c == '2') || (*c == '3'))
		return 1;
	return 2;
}

