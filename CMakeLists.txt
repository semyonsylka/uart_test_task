cmake_minimum_required(VERSION 3.0.0)
project (uart_test)
# find libgpiod

find_library(LIBGPIOD
  NAMES libgpiod.so gpiod 
  HINTS /usr/lib/x86_64-linux-gnu /usr/lib/arm-linux-gnueabihf)

#find_package(libgpiod)

set (SRC_FILES
  modes.h
  modes.c
  device_tools.h
  device_tools.c
  cp_echo_test.c)

add_executable(uart_echo_test ${SRC_FILES})

if (LIBGPIOD)
  target_link_libraries(uart_echo_test gpiod)
  add_definitions(-DLIB_GPIOD_)
endif()

