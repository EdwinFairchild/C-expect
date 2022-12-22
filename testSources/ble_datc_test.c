
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

port_t datcPort;
serial_fd_t datc_fd;
bool verbose;
/*only used for test result results table at the end*/
char projectName[] = "datc Test";

#define DATC_NOT_CONNECTED_TESTS 6
test_t datc_notConnectedTests[DATC_NOT_CONNECTED_TESTS] =
    {
        /* send : expect : attempts : timeout : result : result value*/
        {NULL, ">>> Scanning started <<<",        2, 2000, false},
        {"btn 1 s\n", ">>> Scanning stopped <<<", 2, 2000, false},
        {"btn 1 m\n", "Medium Button 1 Press",    2, 2000, false},
        {"btn 1 l\n", "Clear bonding info",      2, 2000, false},
        {"btn 1 x\n", "XL Button 1 Press",        2, 2000, false},
        {"btn 2 s\n", "No action assigned",    2, 2000, false}};

/*************************************************************************************************/
/*!
 *  \brief  test main entry point
 *
 *  \param  argv[1]     /dev/ttyUSBx  device for BLE_datc
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
    datcPort.fd = datc_fd;
    datcPort.name = argv[1];
    datcPort.verbose = atoi(argv[2]);
    verbose = datcPort.verbose;
    if (initSerial(&datcPort))
    {
        // TODO init failed return meaningful code
        return 1;
    }

    printStartVerbose();
    // TODO : datc is tricky because its starts spamming scann results
    // perhaps just skip the simple comms test
    /* for Dats project this reads all initial ble traffic, also helps flush serial buffer to start tests */
    //singelGenericTest(NULL, ">>> Scanning started <<<", 2, 2000, &datcPort);

    multiGenericTest(datc_notConnectedTests, DATC_NOT_CONNECTED_TESTS, &datcPort); // use this automatically to iterate thourgh tests
 

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
    printTestResults(projectName, datc_notConnectedTests, DATC_NOT_CONNECTED_TESTS);
    printf("> %s ran for %dms\n",projectName, (int)(timeNow()-progStart));
}

/*----------------------------------------------------------------------------------*/