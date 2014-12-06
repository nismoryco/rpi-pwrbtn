/*
 * crpi_gpio.h
 *
 * Raspberry PI C++ GPIO Interface
 * Based off of wiringPi by Gordon Henderson <http://wiringpi.com/>
 *
 */

#ifndef _RPI_GPIO_H
#define _RPI_GPIO_H

#include <stdint.h>

#define	INPUT     0
#define	OUTPUT    1

#define	PUD_OFF   0
#define	PUD_DOWN  1
#define	PUD_UP    2

class crpi_gpio {
	private:
		int get_board_rev();
		int pinToGpio(int p);

		int fd, boardRev;
		uint32_t *gpio;
	public:
		crpi_gpio();
		~crpi_gpio();
		int init_gpio();
		void delayMicroseconds(unsigned int dur);
		void set_pin_mode(int pin, int mode);
		void set_pull_mode(int pin, int pud);
		int read_pin(int pin);
		void write_pin(int pin, int value);
};

#endif
