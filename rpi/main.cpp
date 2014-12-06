/*
 * main.h
 *
 * rpi-pwrbtn
 *
 * A simple daemon to monitor an external power controller for the Raspberry Pi.
 * The daemon monitors certain GPIO pins to determine if a shutdown has been
 * requested by the user pressing the power button connected to the external
 * controller.  After the daemon receives the signal from the external
 * controller, it will initiate a system shutdown.  This program is designed for
 * use with Buildroot (http://www.github.com/nismoryco/buildroot).  This fork of
 * Buildroot includes customizations that I use for my various Raspberry Pi
 * projects.
 *
 * Based on Raspberry Pi - PSU by [gamaral](http://www.github.com/gamaral)
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "main.h"
#include "cprocid.h"
#include "crpi_gpio.h"

static volatile sig_atomic_t run = 1;
crpi_gpio rpi_gpio;

void sig_handler(int signum)
{
	switch (signum)
	{
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		run = 0;
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	/* Check to see if euid is zero */
	if (geteuid() != 0) {
		fprintf(stderr, "Root required\n");
		return 1;
	}

	/* Welcome message */
	fprintf(stdout, "%s\nversion %s\n\n", program, version);

	/* Create a pid file */
	cprocess_id proc_id(PID_FILE);
	if (proc_id.file_exists() == true) {
		fprintf(stderr, "%s is already running\n", program);
		return 1;
	}

	/* Fork into daemon */
	pid_t pid = fork();
	if (pid == -1) {
		fprintf(stderr, "Could not fork\n");
		return 1;
	}
	if (pid > 0)  /* Parent */
		return 0;

	/* Write pid file */
	if (proc_id.write_pid() == false) {
		fprintf(stderr, "%s was unable to create a pid file\n", program);
		return 1;
	}

	/* Install signal handler */
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	/*signal(SIGHUP, sig_handler);*/

	/* Initialize rpi_gpio class */
	int i = rpi_gpio.init_gpio();
	if (i == -1) {
		fprintf(stderr, "unable to initialize gpio\n");
		return 1;
	} else fprintf(stderr, "Raspberry Pi rev %d\n", i);

	/* Close file descriptors */
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	/* shutdown switch */
	rpi_gpio.set_pin_mode(RPI_OUT, OUTPUT);
	rpi_gpio.write_pin(RPI_OUT, 0);
	rpi_gpio.set_pin_mode(RPI_IN, INPUT);

	while (run) {
		sleep(1);
		/* shutdown switch handler */
		if (rpi_gpio.read_pin(RPI_IN) == 1) {
			run = 0;
			system(SYSTEM_POWEROFF_CMD);
		}
	}

	return 0;
}

