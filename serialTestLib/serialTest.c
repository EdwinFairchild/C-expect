#include "serialTest.h"

/* internal helpers */
#define NUM_MAX_SERIAL_PORTS 2

typedef struct 
{
    char *name;
    serial_fd_t fd;
    bool verbose;
}serialPort_t;
serial_fd_t serial_fd[NUM_MAX_SERIAL_PORTS] = {0};
serialPort_t serialPorts[NUM_MAX_SERIAL_PORTS];
int usedPorts = 0;
int numFailedTest = 0;
static bool verboseComms ;
static uint8_t numFailedTests = 0;

/*----------------------------------------------------------------------------------*/
static int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0)
    {
        printf("> ----------| Error from serial port tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;      /* 8-bit characters */
    tty.c_cflag &= ~PARENB;  /* no parity bit */
    tty.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

    tty.c_lflag |= ISIG | ICANON; /* canonical input */
    // tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);

    tty.c_iflag &= ~IGNCR; /* preserve carriage return */
    tty.c_iflag &= ~INPCK;
    tty.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); /* no SW flowcontrol */

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VEOL] = 0;
    tty.c_cc[VEOL2] = 0;
    tty.c_cc[VEOF] = 0x04;
    tty.c_cc[VTIME] = 100;
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("> ----------| Error configuring serial port: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}
/*----------------------------------------------------------------------------------*/
int initSerial(port_t *port)
{
    if(usedPorts >= NUM_MAX_SERIAL_PORTS)
    {
        printf("\n\n---Max number of ports used---\n\n");
        //TODO: return meaningful error code
        return 1;
    }

    verboseComms = port->verbose; 
    port->fd = open(port->name, O_RDWR | O_NOCTTY | O_SYNC);
    if (port->fd < 0)
    {
        printf("Error opening %s: %s\n", port->name, strerror(errno));
        // TODO meaningful error code
        return 255;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(port->fd, B115200);

    usedPorts++;
    return 0;
}
/*----------------------------------------------------------------------------------*/
int serialWrite(const void *msg, uint8_t len , port_t *port)
{
    int wlen;
    wlen = write(port->fd, msg, len);
    if (wlen != len)
    {
        //TODO : return meaningful code
        return 1;
       
    }
    tcdrain(port->fd); /* delay for output */
    return 0;
}
/*----------------------------------------------------------------------------------*/
/*
 * 
 * Passing a NULL message ignores the msg and msgLen only checks if the
 * 'expect' string appears on the terminal during the given timeout: for example
 * checking if the "trimmed 32Khz msg appears"
 * 
 * 
 */
int singelGenericTest(char *msg, char *expect, uint8_t attempts, uint16_t timeout_ms, port_t *port)
{

    unsigned char buf[100];
    unsigned char *p;
    uint8_t testResult = 0;
    bool stillTesting = true;
    int rv;
    fd_set set;
    struct timeval timeout;    
    uint16_t timeStarted = (uint16_t) timeNow();
    int fd = port->fd;
    while(attempts)
    {
        if (msg != NULL){
            serialWrite(msg, strlen(msg),port);
        }
        attempts--;
        
        /* monitor serail port file descriptor and wait for changes */
        do
        {
            FD_ZERO(&set); /* clear the set */
            FD_SET(fd, &set); /* add file descriptor to the set */
            /* per man page: Consider timeout to be undefined after select() returns */
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000* timeout_ms ;
            int readLen;
            
            rv = select(fd + 1, &set, NULL, NULL, &timeout);
            
            /* an error accured */
            if (rv == -1){
                //TODO :  implement meaniful error code
                return 1;

            }
            /* timeout */
            else if (rv == 0){
                stillTesting = false;
                testResult++;          
            }
            /* something to read */
            else{
                readLen = read(fd, buf, sizeof(buf) - 1);
                if (readLen > 0)
                {
                    buf[readLen] = 0;
                    // if verbose enabled
                    if (verboseComms){ 
                        printf("%s\n", buf); 
                    }
                    /* if found a match */
                    if (expect != NULL && strstr(buf, expect) != NULL) {
                        stillTesting = false;
                        testResult = 0; //pass
                        attempts = 0 ; //no need to attempt more
                    }
                    
                    // clear buffer
                    memset(buf, '\0', readLen);
                }
                else if (readLen < 0)
                {
                    // TODO : meaningful error code
                    printf("> ----------| ERROR from read : %d: %s\n", readLen, strerror(errno));
                    return 1;
                }
                // 0 means nothing was read and thats fine
            }
        } while (stillTesting);
        if(attempts)
        {
            //if doing multiple attempts reset variables
            sleep(1);//seconds
            testResult = 0;
            stillTesting = true;
            timeStarted = (uint16_t)timeNow();
            if(verboseComms){
                printf("\n> ---------timed out\n");
            }
        }
        // computer is insanely fast, should wait some time between attempts
        sleep(1); //seconds
    }//attemps

    // computer is insanely fast, should wait some time between tests
    sleep(1); //seconds
    return testResult;
}
/*----------------------------------------------------------------------------------*/
int multiGenericTest(test_t *test, uint8_t numOfTests , port_t *port)
{
    unsigned char buf[100];
    unsigned char *p;
    uint8_t testResult = 0;
    bool stillTesting = true;
    int rv;
    fd_set set;
    struct timeval timeout;    
    uint16_t timeStarted = (uint16_t) timeNow();
    int fd = port->fd;
    //loop through test array

    for (int i = 0; i < numOfTests; i++)
    {
        while(test[i].attempts)
        {
            if (test[i].msg != NULL){
                serialWrite(test[i].msg, strlen(test[i].msg), port);
            }
            test[i].attempts--;
            
            /* monitor serail port file descriptor and wait for changes */
            do
            {
                FD_ZERO(&set); /* clear the set */
                FD_SET(fd, &set); /* add file descriptor to the set */
                /* per man page: Consider timeout to be undefined after select() returns */
                timeout.tv_sec = 0;
                timeout.tv_usec = 1000* test[i].timeout_ms ;
                int readLen;
                
                rv = select(fd + 1, &set, NULL, NULL, &timeout);
                
                /* serial port error accured */
                if (rv == -1){
                    //TODO :  implement meaniful error code
                    return 1;

                }
                /* timeout */
                else if (rv == 0){
                    stillTesting = false;
                    testResult++;          
                }
                /* something to read */
                else{
                    readLen = read(fd, buf, sizeof(buf) - 1);
                    if (readLen > 0)
                    {
                        buf[readLen] = 0;
                        // if verbose enabled
                        if (verboseComms){ 
                            printf("%s\n", buf); 
                        }
                        /* if found a match */
                        if (test[i].expect != NULL && strstr(buf, test[i].expect) != NULL){
                            stillTesting = false;
                            testResult = 0; //pass
                            test[i].attempts = 0 ; //no need to attempt more
                        }
                        
                        // clear buffer
                        memset(buf, '\0', readLen);
                    }
                    /* read error */
                    else if (readLen < 0)
                    {
                        // TODO : meaningful error code
                        return 1;
                    }
                    // 0 means nothing was read and thats fine
                }
            } while (stillTesting);
            // computer is insanely fast, should wait some time between attempts
            if(test[i].attempts)
            {
                //if doing multiple attempts reset variables
                sleep(1); //seconds
                testResult = 0;
                stillTesting = true;
                timeStarted = (uint16_t)timeNow();
                if(verboseComms){
                    printf("\n> ---------timed out\n");
                }
            }
        }//attemps

        // computer is insanely fast, should wait some time between tests
        sleep(1); //seconds
        test[i].result = testResult ;
        stillTesting = true; 
    }//number of test
}
/*----------------------------------------------------------------------------------*/
long long timeNow()
{
    struct timeval te;
    gettimeofday(&te, NULL);                                         // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
    return milliseconds;
}
/*----------------------------------------------------------------------------------*/
void printTestResults(char *testName, test_t *test , int numOfTests )
{
    printf("\n> %s test resutls\n", testName);
    printf("\n.-----------------------------------------.\n");
    printf("|       Test        |       Result        |\n");
    printf("|-----------------------------------------|\n" );
    
    for(int i =0; i < numOfTests; i++)
    {
        if(test[i].msg != NULL){
            int padding = strlen(test[i].msg);
            printf("|      ");
            for(int k = 0 ; k < (padding-1) ; k++)
                printf("%c",test[i].msg[k] );
            for(int j = 0 ; j < 14-padding;j++)
                printf(" ");
        }
        else{
            printf("|      NULL         ");
        }
        if(test[i].result){
            printf("|      **FAIL**       |\n" );
            numFailedTest++;
        }
        else{
            printf("|        PASS         |\n" );
        }
    }
    printf("'-----------------------------------------'\n" );
    
}
/*----------------------------------------------------------------------------------*/
int getNumFailedTests(void){ return numFailedTest;}

void setVerbose(bool verb)
{
    verboseComms = verb;
}