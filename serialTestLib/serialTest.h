#ifndef _SERIAL_TESTER_H_
#define _SERIAL_TESTER_H_


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

/* used to hold the serial port file descriptor in order to allow multiple connections per application */
typedef int serial_fd_t; 

typedef struct 
{
    char *msg;
    char *expect;
    uint8_t attempts;
    uint16_t timeout_ms;
    bool result;

}test_t;

typedef struct {
    char *name;
    serial_fd_t fd;
    bool verbose;

}port_t; 
/*************************************************************************************************/
/*!
 *  \brief  Initializes serial port
 *
 *  \param  serialPort  serial port device passed in to main
 *  \param  verbose     switch for logging serial traffic
 *
 *  \return  error code implying success or failure to init.
 */
/*************************************************************************************************/
int initSerial(port_t *port);

/*************************************************************************************************/
/*!
 *  \brief  Initializes serial port
 *
 *  \param  msg         string to be written to serial port
 *  \param  len         length of string 
 *
 *  \return  error code implying success or failure to write.
 */
/*************************************************************************************************/
int serialWrite(const void *msg, uint8_t len , port_t *port);

int singelGenericTest(char *msg, char *expect, uint8_t attempts,uint16_t timeout_ms,port_t *port);

int multiGenericTest(test_t *test, uint8_t numOfTests ,port_t *port);

long long timeNow();

void printTestResults(char *testName, test_t *test, int numOfTests );

int getNumFailedTests(void);

void setVerbose(bool verb);
#endif
