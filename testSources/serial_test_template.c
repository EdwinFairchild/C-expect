
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

port_t datsPort;
serial_fd_t dats_fd;
bool verbose;
/*only used for test result results table at the end*/
char projectName[] = "example Test";

#define NUM_OF_EXAMPLE_TESTS 3

test_t exampleTests[NUM_OF_EXAMPLE_TESTS] =
    {
        /* test passed value is initlized to false. Timeouts are in ms,
           be mindful of /n and /r being sent back by target"
        /* string to send :      string to expect                  :attempts:timeout:test passed*/
        {"string to send\n", "expected string",                        2, 2000, false},
        {"btn 1 s\n",        ">>> Advertising started <<<",            2, 2000, false},
        {"btn 1 m\n",        "Medium Button 1 Press",                  2, 2000, false}};

/*************************************************************************************************/
/*!
 *  \brief  test main entry point can be modified to do whatever you need
 *
 *  \param  argv[1]     /dev/ttyUSBx  device 
 *  \param  argv[2]     verbose switch for logging serial traffic : 1=verbose : 0= !verbose
 *  \param  ....you can add more params as needed, dont forget to add them to launcher script
 *  \return  TODO: error code : number of failed test or code implying some failure  
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
    /* you can send a single test with NULL NULL just to print/flush any initial serial traffic */
    singelGenericTest(NULL, NULL, 1, 2000, &datsPort);
   
    /*  basic serial port test to see if we can talk to the device */
    if (singelGenericTest("gibberish\n", "ERROR: unrecognized command \"gibberish\"\r\n", 2, 5000, &datsPort))
    {
        printf("> ------- Basic serial comms test failed -------");
        return 1;
    }
    
    multiGenericTest(exampleTests, NUM_OF_EXAMPLE_TESTS, &datsPort); 
    endTesting();
    return numFailedTests;
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
    printTestResults(projectName, exampleTests, NUM_OF_EXAMPLE_TESTS);
    printf("> %s ran for %dms",projectName, (int)(timeNow()-progStart));
}
