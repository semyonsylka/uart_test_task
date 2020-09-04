#ifndef MODES_H
#define MODES_H

#define BUF_SIZE 512

/// echo mode method for configured tty device
int echo_mode(int fd);

int trigger_mode();

#endif