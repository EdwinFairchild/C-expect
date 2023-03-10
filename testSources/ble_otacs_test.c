
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

port_t otasPort;
port_t otacPort;
serial_fd_t otas_fd;
serial_fd_t otac_fd;
bool verbose;
enum test
{
    phySwitching,
    fileDiscovery,
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
   
    otasPort.fd = otas_fd;
    otasPort.name = argv[1];
    otasPort.verbose = atoi(argv[3]);

    otacPort.fd = otac_fd;
    otacPort.name = argv[2];
    otacPort.verbose = atoi(argv[3]);
    
    verbose = otacPort.verbose;
    if (initSerial(&otacPort))
    {
        // TODO init failed return meaningful code
        printf("\n\n--------------Serial Ports failed to init--------------\n\n");
        return 1;
    }

    printStartVerbose();
    /* flush serial buffer */
    singelGenericTest(NULL, NULL, 1, 2000, &otacPort);
   
    int testCounter=0;

    /* PHY switch request test */
    if(verbose)
        printf("\n>------ otac -----\n");
    testResults[testCounter] = singelGenericTest("btn 1 x\n", "DM_PHY_UPDATE_IND - RX: 2, TX: 2", 2, 3000, &otacPort);
        testCounter++;

    /* Discover file */
    if(verbose)
        printf("\n>------ otac -----\n");
    testResults[testCounter] = singelGenericTest("btn 2 s\n", ">>> File discovery complete <<<", 2, 3000, &otacPort);
    testCounter++;

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
        printf("\n\n??????----------| BLE end verbose output |----------??????\n\n");
    }
}
static void printTable(void)
{
    //make a dummy test to pass to the fucntion
    test_t datc_notConnectedTests[testCount] =
    {
        {"phy switchingn", NULL, 0, 0, (bool)testResults[0]},
        {"file disc.\n", NULL, 0, 0, (bool)testResults[1]}};
    
    printTestResults("datc/s connected",datc_notConnectedTests, testCount);
}
/*----------------------------------------------------------------------------------*/
static void endTesting(void)
{
    printEndVerbose(); 
    printTable();
    printf("> BLE_otac/s ran for %dms\n\n", (int)(timeNow()-progStart));
}

/*----------------------------------------------------------------------------------*/