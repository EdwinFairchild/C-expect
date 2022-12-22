
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
static void printTable(void);

port_t datsPort;
port_t datcPort;
serial_fd_t dats_fd;
serial_fd_t datc_fd;
bool verbose;
enum test
{
    simpleMsg,
    phySwitching,
    transferSpeed,
    testCount
};

int testResults[testCount] ={0};

/*************************************************************************************************/
/*!
 *  \brief  test main entry point
 *
 *  \param  argv[1]     /dev/ttyUSBx  device for BLE_dats
 *  \param  argv[2]     /dev/ttyUSBx  device for BLE_datc
 *  \param  argv[3]     verbose switch for logging serial traffic : 1 : 0
 *
 *  \return  error code implying success or failure to init.
 */
/*************************************************************************************************/
int main(int argc, char **argv)
{
    
    if (argv[1] == NULL || argv[2] == NULL)
    {
        // No serial port provided
        // TODO return meaningful code
        return 1;
    }
    progStart = timeNow(); //program start timestamp
   
    // datsPort.fd = dats_fd;
    // datsPort.name = argv[1];
    // datsPort.verbose = (bool)atoi(argv[3]);

    datcPort.fd = datc_fd;
    datcPort.name = argv[2];
    datcPort.verbose = (bool)atoi(argv[3]);
    
    verbose = datcPort.verbose;
    if (initSerial(&datcPort))
    {
        // TODO init failed return meaningful code
        printf("\n\n--------------Serial Ports failed to init--------------\n\n");
        return 1;
    }
    
    printStartVerbose();
    /* flush serial buffer */
    singelGenericTest(NULL, ">>> Scanning started <<<", 2, 2000, &datcPort);
    int testCounter=0;

    /* send siple message test */
    if(verbose)
        printf("\n>------ Datc -----\n");
    testResults[testCounter++] = singelGenericTest("btn 2 l\n", "hello back", 1, 2000, &datcPort);
    
    /* PHY switch request test */
    if(verbose)
        printf("\n>------ Datc -----\n");
    testResults[testCounter++] = singelGenericTest("btn 2 s\n", "DM_PHY_UPDATE_IND - RX: 2, TX: 2", 1, 2000, &datcPort);

    /* PHY switch request test */
    if(verbose)
        printf("\n>------ Datc transfer speed test turning off verbose -----\n");
    setVerbose(false);
    testResults[testCounter] = singelGenericTest("btn 2 x\n", "bits transferred in", 1, 2000, &datcPort);
   
    endTesting();
    return getNumFailedTests();;
}
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
static void printTable(void)
{
    //make a dummy test to pass to the fucntion
    test_t datc_notConnectedTests[testCount] =
    {
        {"simple msg\n", NULL, 0, 0, (bool)testResults[0]},
        {"phy switching\n", NULL, 0, 0, (bool)testResults[1]},
        {"xfer speed\n", NULL, 0, 0, (bool)testResults[2]}};
    
    printTestResults("datc/s connected",datc_notConnectedTests, testCount);
}
/*----------------------------------------------------------------------------------*/
static void endTesting(void)
{
    printEndVerbose(); 
    printTable();
    printf("> BLE_datc/s ran for %dms\n", (int)(timeNow()-progStart));
}

/*----------------------------------------------------------------------------------*/