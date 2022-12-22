
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

port_t fitPort;
serial_fd_t fit_fd;
bool verbose;
/*only used for test result results table at the end*/
char projectName[] = "fit Test";

#define NUM_OF_EXAMPLE_TESTS 2

test_t fitTests[NUM_OF_EXAMPLE_TESTS] =
    {
        /* test passed value is initlized to false. Timeouts are in ms,
           be mindful of /n and /r being sent back by target"
        /* string to send :      string to expect                  :attempts:timeout:test passed*/
        {"btn 1 s\n",        "Short Button 1 Press",           2, 2000, false},
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
    fitPort.fd = fit_fd;
    fitPort.name = argv[1];
    fitPort.verbose = atoi(argv[2]);
    verbose = fitPort.verbose;
    if (initSerial(&fitPort))
    {
        // TODO init failed return meaningful code
        return 1;
    }

    printStartVerbose();
    /* you can send a single test with NULL NULL just to print/flush any initial serial traffic */
    singelGenericTest(NULL, NULL, 1, 2000, &fitPort);
   
    /*  basic serial port test to see if we can talk to the device */
    if (singelGenericTest("gibberish\n", "ERROR: unrecognized command \"gibberish\"\r\n", 2, 5000, &fitPort))
    {
        printf("> ------- Basic serial comms test failed -------");
        return 1;
    }
    
    multiGenericTest(fitTests, NUM_OF_EXAMPLE_TESTS, &fitPort); 
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
    printTestResults(projectName, fitTests, NUM_OF_EXAMPLE_TESTS);
    printf("> %s ran for %dms",projectName, (int)(timeNow()-progStart));
}
