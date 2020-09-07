#include "modes.h"
#include "device_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

/// echo mode method
int echo_mode(int fd) {
  // flag for termination
  int exit_flag = 0;
  // buffer stdin
  char inputbuf[BUF_SIZE];
  // buffer for read from tty
  char receivedbuf[BUF_SIZE];
  int rec_cnt, ret_val;

  while (!exit_flag) {
    // clear buffers
    memset(&inputbuf, 0, BUF_SIZE);
    memset(&receivedbuf, 0, BUF_SIZE);

    printf("Input data to send or print exit to exit\n");
    scanf("%s", inputbuf);
    if (strcmp("exit", inputbuf) == 0) {
      printf("\nexit...\n");
      exit_flag = 1;
      break;
    }

    // write data to uart
    printf("\nwriting string len is %i\n\n", strlen(inputbuf));
    if (write(fd, inputbuf, strlen(inputbuf)) != strlen(inputbuf)) {
      printf("Error %i when write to device: %s\n", errno, strerror(errno));
    }
    // read data from uart
    printf("reading data...\n");
    do {
      if (ioctl(fd, FIONREAD, &rec_cnt) < 0)
        printf("ioctl() FIONREAD request failed\n");
    } while (rec_cnt == 0);
    rec_cnt = read(fd, receivedbuf, BUF_SIZE);
    if (rec_cnt < 0) {
      printf("Error %i when read from device: %s\n", errno, strerror(errno));
    } else {
      printf("Received data (len %d) from uart: %s\n\n\n", rec_cnt,
             receivedbuf);
    }
  }
  return 0;
}

#ifdef LIB_GPIOD_
int trigger_mode(int uart_fd, int gpio_line_number, 
                 int seconds_to_wait) {
  // setting gpio resources
  struct gpiod_chip *chip;
  struct gpiod_line *line;
  struct gpiod_line_event event;
  struct timespec ts = {seconds_to_wait, 0};
  //message to uart
  char *msg = "Hello, from Rpi";
  
  int ret = 0;
  ret = configure_gpio(&line, &chip, gpio_line_number);
  if (ret) {
    printf("Gpio configuring failed\n");
    return 1;
  }

  while (1) {
    ret = gpiod_line_event_wait(line, &ts);
    if (ret < 0) {
      printf("Error on wait event %d\n", ret);
      break;
    }
    if (ret == 0) {
        printf("Wait for notification timeout\n");
        break;
    }
    ret = gpiod_line_event_read(line, &event);
    if (ret < 0) {
      printf("Error while reading event");
      break;  
    }
    printf("Event notify received. Sending to uart\n");
    if (write(uart_fd, msg, strlen(msg)) != strlen(msg)) {
        printf("Write to uart failed\n");
        break;
    }
  }  
  release_gpio(&line, &chip);
  return 0;
}
#else
int trigger_mode(int uart_fd, int gpio_line_number, 
                 int seconds_to_wait) {
  return 0;
}
#endif