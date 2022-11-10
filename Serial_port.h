#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

/*@brief: Starts the serial port
*
* @param portname: name of the port as shown in the terminal list
* 
* @param baudrate: fixed baudrates from 1200 to 38400
*
* @param parity fixed to even for now
*
*/
int serial_port_init(char *port_name, unsigned int baudrate, unsigned int parity);


/*@brief: Sends a string to the port 
*
*/
void serial_port_write(char* msg);

/*@brief: Reads a string from the port 
*
*/
size_t serial_port_read(char *msg,size_t length);


void serial_port_close(void);

#endif //SERIAL_PORT_H