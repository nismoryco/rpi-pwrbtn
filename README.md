#rpi-pwrbtn

A simple daemon to monitor an external power controller for the Raspberry Pi.
The daemon monitors certain GPIO pins to determine if a shutdown has been
requested by the user pressing the power button connected to the external
controller.  After the daemon receives the signal from the external
controller, it will initiate a system shutdown.  This program is designed for
use with [Buildroot](http://www.github.com/nismoryco/buildroot).  This fork of
Buildroot includes customizations that I use for my various Raspberry Pi
projects.

###Based on Raspberry Pi - PSU by [gamaral](http://www.github.com/gamaral)

