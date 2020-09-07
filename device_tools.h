#ifndef DEVICE_CONF_H
#define DEVICE_CONF_H

/*!
    \brief Method configuring tty device
    \param tty_fd - opened file descriptor of tty device
    \param baudrate_code - termios-defined code for speed of connection
    \param vmin - .c_cc[VMIN] value to set
    \param vtime - .c_cc[VTIME] value to set
    \return 0 if all OK, 1 - otherwise
*/
int configure_tty(int tty_fd, int baudrate_code, 
                  int vmin, int vtime); 

//

int get_default_baudrate();

/// method finds nearest or exact baudrate
int check_baudrate_or_find_nearest(int br);

// gpiod 
#ifdef LIB_GPIOD_
#include <gpiod.h>

/// method opens chip and line (chipname hardcoded)
int configure_gpio(struct gpiod_line **line,
                   struct gpiod_chip **chip,            
                   int line_number);

/// method releases the gpio resources
void release_gpio(struct gpiod_line **line,
                 struct gpiod_chip **chip);

#endif


#endif