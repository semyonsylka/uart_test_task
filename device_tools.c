#include "device_tools.h"

#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

// define baudrates
const int possible_baudrates[] = {50,   75,   110,   134,   150,   200,
                                  300,  600,  1200,  1200,  1800,  2400,
                                  4800, 9600, 19200, 38400, 57600, 115200};
const int possible_baudrates_codes[] = {
    B50,   B75,   B110,  B134,  B150,  B200,   B300,   B600,   B1200,
    B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200};

int configure_tty(int tty_fd, int baudrate_code, int vmin, int vtime) {
  // check for tty
  if (!isatty(tty_fd)) {
    printf("Device not a tty!\n");
    return 1;
  }

  struct termios tty;
  // read existing settings
  if (tcgetattr(tty_fd, &tty) != 0) {
    printf("Error %i when getting attributes: %s\n", errno, strerror(errno));
    return 1;
  }

  // setting baudrates
  cfsetispeed(&tty, baudrate_code);
  cfsetospeed(&tty, baudrate_code);

  tty.c_cc[VTIME] = vtime;
  tty.c_cc[VMIN] = vmin;

  // apply changes
  if (tcsetattr(tty_fd, TCSANOW, &tty) != 0) {
    printf("Error %i when setting attributes: %s\n", errno, strerror(errno));
    return 1;
  }
  return 0;
}

#ifdef LIB_GPIOD_
#define CONSUMER "CONSUMER"
/// configure gpio
int configure_gpio(struct gpiod_line **line,
                   struct gpiod_chip **chip,                   
                   int line_number) {
  char *chipname = "gpiochip0";
  // open chip
  *chip = gpiod_chip_open_by_name(chipname);
  if (!*chip) {
    printf("Open chip failed\n");
    return 1;
  }
  // open line
  *line = gpiod_chip_get_line(*chip, line_number);
  if (!*line) {
    printf("Open %i line failed\n", line_number);
    return 1;
  } 
  // if (gpiod_line_request_input(line, CONSUMER) != 0) {
  //   printf("Request input failed\n");
  // }
  // int val = gpiod_line_get_value(line);
  // printf("Line value is:%d\n", val);
  // set event
  int event_set = gpiod_line_request_both_edges_events(*line, "line_ev_consumer");
  if (event_set < 0) {
    printf("Set %i line event failed\n", line_number);
    return 1;
  }
  return 0;
}

void release_gpio(struct gpiod_line **line,
                 struct gpiod_chip **chip) {           
  gpiod_line_release(*line);
  gpiod_chip_close(*chip);  
}

#endif

int get_default_baudrate() {
    return B1152000;
}

int check_baudrate_or_find_nearest(int br) {
  int diff, min_diff, prev_diff, result;
  int br_count = sizeof(possible_baudrates) / sizeof(int);
  for (int i = 0; i < br_count; i++) {
    diff = abs(possible_baudrates[i] - br);
    if (diff < min_diff || i == 0) {
      min_diff = diff;
      result = possible_baudrates_codes[i];
    }
  }
  return result;
}