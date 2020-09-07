#include <argp.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/ioctl.h>
// #include <termios.h>
#include <unistd.h>
// #include <gpiod.h>
#include "modes.h"
#include "device_tools.h"

enum messagemode {
  echo = 0,
  trigger = 1,
};

// parsing args
struct arguments {
  int baudrate;
  char *device;
  int gpio_pin;
  enum messagemode mode;
};
// argp arguments init
static struct argp_option options[] = {
    {"baudrate", 'b', "Baudrate", 0, "Uart connection baudrate"},
    {"device", 'd', "DevicePath", 0, "Uart device (ex. /dev/ttyUSB0)"},
    {"mode", 'm', "Mode", 0, "Server mode ('echo' or 'trigger')"},
    {"pin", 'p', "Pin_number", 0, "GPIO pin number for trigger mode"},
    {0}};

// define max gpio pin number (according to rpi 4b)
const int max_gpio_pin_number = 27;

/// check for number method
int isnumber(const char *c) {
  for (int i = 0; c[i] != 0; i++) {
    if (!isdigit(c[i]))
      return 0;
  }
  return 1;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
  case 'b':
    if (isnumber(arg)) {
      arguments->baudrate = check_baudrate_or_find_nearest(atoi(arg));
    } else {
      printf("Input baudrate not a number! Set default value (115200)\n");
      arguments->baudrate = get_default_baudrate();
    }
    break;
  case 'd':
    // printf("%s",arg);
    if (access(arg, F_OK) != -1) {
      // file exists
      arguments->device = arg;
    }
    break;
  case 'm':
    if (strcmp("echo", arg) == 0)
      arguments->mode = echo;
    else if (strcmp("trigger", arg) == 0)
      arguments->mode = trigger;
    else // other modes will multiplexing here
      arguments->mode = echo;
    break;
  case 'p':
    if (isnumber(arg)) {
      arguments->gpio_pin = atoi(arg);
    } else {
      arguments->gpio_pin = -2;
    }
    break;  
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

// define argp
static const struct argp argp = {options, parse_opt, NULL, NULL};

/*!
    \brief Method configuring tty device
    \param tty_fd - opened file descriptor of tty device
    \param baudrate_code - termios-defined code for speed of connection
    \param vmin - .c_cc[VMIN] value to set
    \param vtime - .c_cc[VTIME] value to set
    \return 0 if all OK, 1 - otherwise
*/

/// validate arguments method
int check_args(const struct arguments *args) {
  // check for device
  if (strcmp("", args->device) == 0) {
    printf("Device not founded!\n");
    return 1;
  }
  // check for pin
  if (trigger == args->mode) {
    if (args->gpio_pin == -1) {
      // pin not setted
      printf("Pin not setted for mode: trigger\n");
      return 1;
    } else {
      // pin setted
      // check pin in range
      if (args->gpio_pin < 0 || args->gpio_pin > max_gpio_pin_number) {
        printf("Pin incorrect\n");
        return 1;
      }
    }
  }    
  return 0;
}

int main(int argc, char *argv[]) {
  // status of executed methods
  int err_code = 0;
  
  // parsing args
  struct arguments args;
  args.baudrate = 0;
  args.device = "";
  args.gpio_pin = -1;
  args.mode = 0;

  argp_parse(&argp, argc, argv, 0, 0, &args);
  // check arguments
  err_code = check_args(&args);
  if (err_code) {
    printf("Some of arguments incorrect\n");
    return err_code;
  }

  // open serial port
  int uart_fd = open(args.device, O_RDWR);
  if (uart_fd < 0) {
    printf("Error %i when opening serial port: %s\n", errno, strerror(errno));
    return 1;
  }

  err_code = configure_tty(uart_fd, args.baudrate, 0, 10);
  if (err_code != 0) {
    // occurs some troubles while
    return err_code;
  }

  // start the mode
  switch (args.mode) {
  case echo:
    err_code = echo_mode(uart_fd);
    break;
  case trigger:
    err_code = trigger_mode(uart_fd, args.gpio_pin, 60);
    break;
  default:
    err_code = echo_mode(uart_fd);
    break;
  }

  close(uart_fd);

  return err_code;
}