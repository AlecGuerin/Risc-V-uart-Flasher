#include "Serial_port.h"

static int serial_port;

int serial_port_init(char *port_name, unsigned int baudrate, unsigned int parity)
{

    unsigned int baudrate_enum = 0;

    serial_port = open(port_name, O_RDWR);

    if (serial_port < 1)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return 1;
    }

    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    switch (parity)
    {
    case 1200:
        baudrate_enum = B1200;
        break;
    case 2400:
        baudrate_enum = B2400;
        break;
    case 4800:
        baudrate_enum = B4800;
        break;
    case 9600:
        baudrate_enum = B9600;
        break;
    case 19200:
        baudrate_enum = B19200;
        break;
    case 38400:
        baudrate_enum = B38400;
        break;
    default:
        baudrate_enum = B9600;
        break;
    }

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, baudrate_enum);
    cfsetospeed(&tty, baudrate_enum);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    return 0;
}

void serial_port_write(char *msg)
{
    write(serial_port, msg, strlen(msg));
}

void serial_port_write_byte(char msg)
{
    write(serial_port, &msg, 1);
}

void serial_port_write_data(char *msg, int size)
{
    write(serial_port, msg, size);
}

size_t serial_port_read(char *msg,size_t length)
{
    return read(serial_port, msg, length);
}

void serial_port_close(void)
{
    close(serial_port);
}
