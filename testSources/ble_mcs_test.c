
#include <sys/select.h>
#include "../serialTestLib/serialTest.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int progStart, timeoutStart;

uint8_t numFailedTests = 0;
// prototypes
static void endTesting(void);
static void printStartVerbose(void);
static void printEndVerbose(void);

port_t mcsPort;
serial_fd_t mcs_fd;
bool verbose;

/* TODO ************ This test is not finished */

#define NUM_OF_TESTS 3
test_t mcs_notConnectedTest[NUM_OF_TESTS] =
    {
        /* test passed value is initlized to false. Timeouts are in ms*/
        /* string to send :      string to expect                  :attempts:timeout:test passed*/
        {"btn 1 s\n",        "Short Button 1 Press",         2, 2000, false},
        {"btn 1 m\n",        "Medium Button 1 Press",        2, 2000, false},
        {"btn 1 l\n",        "Long Button 1 Press",          2, 2000,false}};

/*************************************************************************************************/
/*!
 *  \brief  test main entry point can be modified to do whatever you need
 *
 *  \param  argv[1]     /dev/ttyUSBx  device for BLE_dats
 *  \param  argv[2]     verbose switch for logging serial traffic : 1 : 0
 *
 *  \return  error code implying success or failure to init.
 */
/*************************************************************************************************/
int main(int argc, char **argv)
{
    
    if (argv[1] == NULL)
    {
        // No serial port provided
        // TODO return meaningful code
        return 1;
    }
    progStart = timeNow(); //program start timestamp
    mcsPort.fd = mcs_fd;
    mcsPort.name = argv[1];
    mcsPort.verbose = atoi(argv[2]);
    verbose = mcsPort.verbose;
    if (initSerial(&mcsPort))
    {
        // TODO init failed return meaningful code
        return 1;
    }

    printStartVerbose();
    /* you can send a single test with NULL NULL just to print/flush any initial serial traffic */
    singelGenericTest(NULL, NULL, 2, 3000, &mcsPort);
    
    /*  basic serial port test to see if we can talk to the device */
    if (singelGenericTest("gibberish\n", "ERROR: unrecognized command \"gibberish\"\r\n", 2, 5000, &mcsPort))
    {
        printf("> ------- Basic serial comms test failed -------");
        return 1;
    }
    multiGenericTest(mcs_notConnectedTest, NUM_OF_TESTS, &mcsPort); 
    endTesting();
    return getNumFailedTests();;
}
/*----------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------*/
static void printStartVerbose(void)
{
    if (verbose)
    {
        printf("\n\n>>----------| BLE start verbose output |----------<<\n\n");
    }
}
/*----------------------------------------------------------------------------------*/
static void printEndVerbose(void)
{
    if (verbose)
    {
        printf("\n\n>>----------| BLE end verbose output |----------<<\n\n");
    }
}
/*----------------------------------------------------------------------------------*/
static void endTesting(void)
{
    printEndVerbose();
    printTestResults("BLE_dats", mcs_notConnectedTest, NUM_OF_TESTS);
    printf("> BLE_dats ran for %dms", (int)(timeNow()-progStart));
}
