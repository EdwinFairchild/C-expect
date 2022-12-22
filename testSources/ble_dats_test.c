
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
// vars
int progStart, timeoutStart;

uint8_t numFailedTests = 0;
// prototypes
static void endTesting(void);
static void printStartVerbose(void);
static void printEndVerbose(void);
static void whenDisconnectedTests(void);
static void whenConnectedTests(void);

port_t datsPort;
serial_fd_t dats_fd;
bool verbose;

/* TODO ************ This test is not finished */

#define DATC_NOT_CONNECTED_TESTS 6
#define DATS_NOT_CONNECTED_TESTS 5
#define NUM_OF_CONNECTED_TESTS 0

test_t dats_notConnectedTests[DATS_NOT_CONNECTED_TESTS] =
    {
        /* send : expect : attempts : timeout : result : result value */
        {"btn 2 s\n", ">>> Advertising stopped <<<",       2, 2000, false},
        {"btn 1 s\n", ">>> Advertising started <<<",      2, 2000, false},
        {"btn 1 m\n", "Medium Button 1 Press",             2, 2000, false},
        {"btn 1 x\n", "XL Button 1 Press\r\n",             2, 2000, false},
        {"btn 1 l\n", "Clear resolving list status 0x00", 2, 2000, false}};
/*************************************************************************************************/
/*!
 *  \brief  test main entry point
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
    datsPort.fd = dats_fd;
    datsPort.name = argv[1];
    datsPort.verbose = atoi(argv[2]);
    verbose = datsPort.verbose;
    if (initSerial(&datsPort))
    {
        // TODO init failed return meaningful code
        return 1;
    }

    printStartVerbose();
    /* for Dats project this reads all initial ble traffic, also helps flush serial buffer to start tests */
    singelGenericTest(NULL, NULL, 1, 2000, &datsPort);
   
    // Dats basic serial port test to see if we can talk to the device
    if (!singelGenericTest("gibberish\n", "ERROR: unrecognized command \"gibberish\"\r\n", 2, 5000, &datsPort))
    {
        multiGenericTest(dats_notConnectedTests, DATS_NOT_CONNECTED_TESTS, &datsPort); // use this automatically to iterate thourgh tests
    }
    else
    {
        printf("> ------- Basic serial comms test failed -------");
        numFailedTests++;
        return 1;
    }

    endTesting();
    return getNumFailedTests();
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
        printf("\n\n≫≫----------| BLE end verbose output |----------≪≪\n\n");
    }
}
/*----------------------------------------------------------------------------------*/
static void endTesting(void)
{
    printEndVerbose();
    printTestResults("BLE_dats", dats_notConnectedTests, DATS_NOT_CONNECTED_TESTS);

    printf("> BLE_dats ran for %dms\n", (int)(timeNow()-progStart));
}

/*----------------------------------------------------------------------------------*/