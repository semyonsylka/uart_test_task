#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <argp.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#define BUF_SIZE 512

enum messagemode
{
    echo = 0,
};

// parsing args
struct arguments
{
    int baudrate;
    char* device;
    enum messagemode mode;
};
// argp arguments init
static struct argp_option options[] = 
{
    {"baudrate", 'b', "Baudrate", 0, "Uart connection baudrate"},
    {"device", 'd', "DevicePath", 0, "Uart device (ex. /dev/ttyUSB0)"},
    {"mode", 'm', "Mode", 0, "Server mode (only 'echo')"},
    {0}
};

/// check for number method
int isnumber(const char * c)
{
    for (int i = 0; c[i] != 0; i++)
    {
        if (!isdigit(c[i]))
            return 0;
    }
    return 1;
}

static error_t
parse_opt (int key, char* arg, struct argp_state *state)
{
    struct arguments* arguments = state->input;

    switch (key)
    {
        case 'b':
        if (isnumber(arg))
        {
            arguments->baudrate = check_baudrate_or_find_nearest(atoi(arg));            
        }
        else
        {
            printf("Input baudrate not a number! Set default value (115200)\n");
            arguments->baudrate = B115200;
        }
        break;
        case 'd':
        // printf("%s",arg);
        if (access(arg, F_OK) != -1)
        {
            // file exists
            arguments->device = arg;
        }
        break;
        case 'm':
        if (strcmp("echo", arg) == -1)
            arguments->mode = echo;
        else // other modes will multiplexing here
            arguments->mode = echo;        
        break;
        default:
            return ARGP_ERR_UNKNOWN;                   
    }
    return 0;
}

// define argp 
static const struct argp argp = {options, parse_opt, NULL, NULL};

// define baudrates
const int possible_baudrates[] = {50, 75, 110, 134, 150, 200, 300, 600, 1200,
                            1200, 1800, 2400, 4800, 9600, 19200, 38400,
                            57600, 115200};
const int possible_baudrates_codes[] = {B50, B75, B110, B134, B150, B200, B300, B600, B1200,
                                  B1200, B1800, B2400, B4800, B9600, B19200, B38400,
                                  B57600, B115200};

/// method finds nearest or exact baudrate
int check_baudrate_or_find_nearest(int br)
{
    int diff, min_diff, prev_diff, result;
    int br_count = sizeof(possible_baudrates)/sizeof(int);
    for (int i = 0; i < br_count; i++)
    {
        diff = abs(possible_baudrates[i] - br);
        if (diff < min_diff || i == 0)
        {
            min_diff = diff;
            result = possible_baudrates_codes[i];
        }
    }
    return result;
}

/// echo mode method
int echo_mode(int fd)
{
    // flag for termination
    int exit_flag = 0;
    // buffer stdin
    char inputbuf[BUF_SIZE];
    // buffer for read from tty
    char receivedbuf[BUF_SIZE];
    int rec_cnt, ret_val;
        
    while (!exit_flag)
    {
        // clear buffers
        memset(&inputbuf, 0, BUF_SIZE);
        memset(&receivedbuf, 0, BUF_SIZE);

        printf("Input data to send or print exit to exit\n");
        scanf("%s",inputbuf);
        if (strcmp("exit",inputbuf) == 0)
        {
            printf("\nexit...\n");
            exit_flag = 1;
            break;
        }

        // write data to uart
        printf("\nwriting string len is %i\n\n", strlen(inputbuf));
        if(write(fd, inputbuf, strlen(inputbuf)) != strlen(inputbuf))
        {
            printf("Error %i when write to device: %s\n", errno, strerror(errno));
        }
        // read data from uart
        printf("reading data...\n");        
        do
        {
            if (ioctl(fd, FIONREAD, &rec_cnt) < 0)
                printf("ioctl() FIONREAD request failed\n");     
        } while (rec_cnt == 0);           
        rec_cnt = read(fd, receivedbuf, BUF_SIZE);
        if (rec_cnt < 0)
        {
            printf("Error %i when read from device: %s\n", errno, strerror(errno));
        }
        else
        {
            printf("Received data (len %d) from uart: %s\n\n\n", rec_cnt, receivedbuf);        
        }                
       }
       return 0;
}

/*!
    \brief Method configuring tty device
    \param tty_fd - opened file descriptor of tty device
    \param baudrate_code - termios-defined code for speed of connection
    \param vmin - .c_cc[VMIN] value to set
    \param vtime - .c_cc[VTIME] value to set
    \return 0 if all OK, 1 - otherwise
*/
int configure_tty(int tty_fd, int baudrate_code, int vmin, int vtime)
{
        // check for tty
    if (!isatty(tty_fd))
    {
        printf("Device not a tty!\n");
        return 1;
    }

    struct termios tty;
    // read existing settings
    if (tcgetattr(tty_fd, &tty) != 0)
    {
        printf("Error %i when getting attributes: %s\n", errno, strerror(errno));
        return 1;
    }

    // setting baudrates
    cfsetispeed(&tty, baudrate_code);
    cfsetospeed(&tty, baudrate_code);

    tty.c_cc[VTIME] = vtime;
    tty.c_cc[VMIN] = vmin;

    // apply changes
    if (tcsetattr(tty_fd, TCSANOW, &tty) != 0)
    {
        printf("Error %i when setting attributes: %s\n", errno, strerror(errno));
        return 1;
    }
    return 0;
}


int main(int argc, char * argv[])
{
    // parsing args
    struct arguments args;
    args.baudrate = 0;
    args.device = "";
    args.mode = 0;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    if (strcmp("",args.device) == 0)
    {
        printf("Device not founded!\n");
        return 1;
    }
    
    // open serial port
    int uart_fd = open(args.device, O_RDWR);
    if (uart_fd < 0)
    {
        printf("Error %i when opening serial port: %s\n", errno, strerror(errno));
        return 1;
    }

    // status of executed methods
    int err_code = 0;

    err_code = configure_tty(uart_fd, args.baudrate, 0, 10);
    if(err_code != 0)
    {
        // occurs some troubles while
        return err_code;
    }

    // start the mode    
    switch (args.mode)
    {
    case echo:
        err_code = echo_mode(uart_fd);
        break;    
    default:
        err_code = echo_mode(uart_fd);
        break;
    } 
     
    close(uart_fd);

    return err_code;
}