# Basic uart usage programm

## Build

Create build directory and run cmake in them with path to directory with CMakeLists.txt

```bash
mkdir build
cd build
cmake /path/to/uart_test_task
make
```

## Usage

Connect uart bridge, find his tty-file location and run uart_echo_test.
Example:
```bash
./uart_echo_test -d /dev/ttyUSB0 -b 115200 -m echo
```
Print --help arg to executable to find information about used flags
```bash
./uart_echo_test --help
```

## Requires
cmake, gcc
