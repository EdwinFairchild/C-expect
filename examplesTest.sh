#!/bin/bash
function script_clean_up()
{
    kill -9 $openocd_dapLink1_pid
    kill -9 $openocd_dapLink2_pid
}
trap script_clean_up  EXIT SIGINT

# TODO: parameterize these so they are not redundant 
function start_openocd_DAPLINK_1()
{
    openocd -f $OPENOCD_TOOL_PATH/interface/cmsis-dap.cfg -f $OPENOCD_TOOL_PATH/target/$TARGET_1_CFG -s $OPENOCD_TOOL_PATH  -c "cmsis_dap_serial  ${CMSIS_DAP_ID_1}" -c "gdb_port 3333" -c "telnet_port 4444" -c "tcl_port 6666"  &
    openocd_dapLink1_pid=$!
}
function start_openocd_DAPLINK_2()
{
    openocd  -f $OPENOCD_TOOL_PATH/interface/cmsis-dap.cfg -f $OPENOCD_TOOL_PATH/target/$TARGET_2_CFG -s $OPENOCD_TOOL_PATH  -c "cmsis_dap_serial  ${CMSIS_DAP_ID_2}" -c "gdb_port 3334" -c "telnet_port 4445" -c "tcl_port 6665" &
    openocd_dapLink2_pid=$!
}

#function accepts following arguments:
#      $1                $2          $3
# /ble_mcs_test  "/dev/ttyUSB1" $VERBOSE_TEST  
# TODO parameterize port so this can turn into one function
function run_notConntectedTest()
{
    project_marker
    cd $PROJECT_NAME
    echo "> Building $PROJECT_NAME"
    set +x
    #$(make -j8)
    echo skipping build
    if [ $? -eq 0 ]; then
        cd build/
        #start_openocd_DAPLINK_1
        echo "> Flashing $PROJECT_NAME"
        arm-none-eabi-gdb $TARGET_1_LC.elf -ex "target extended-remote localhost:3333"  -ex "monitor reset halt" -ex load  -ex "monitor reset run" -ex disconnect -ex quit > /dev/null
        cd $MSDK_DIR/Examples_tests
        echo "> Testing $PROJECT_NAME"
        #runs desired test 
        $1  $2 $3 
        # update failed test count
        let "numOfFailedTests+=$?"       
        #get back to target directory
        cd $MSDK_DIR/Examples/$TARGET_1_UC
    else
        printf "\n\n━━━━━━ BUILD FAILED. TEST ABORTED ━━━━━━ \n\n"
    fi
}

function project_marker()
{
    printf "\r\n ━━━━━━━━━━━━━━━━━━━━━━━|Starting $PROJECT_NAME|━━━━━━━━━━━━━━━━━━━━━━━ \r\n\r\n"
}
#--------------------------------------------------------------------------------------------
#keep track of directory count, might need it
projIdx=1
numOfFailedTests=0
#no filter would support ALL projects not just ble
project_filter='BLE_'

# testing ME17 only for now 
cd Examples/$TARGET_1_UC
#used for all the not connected test
start_openocd_DAPLINK_1

# connect to helper mcu and halt
# in case it is running a client/server that will try to connect
# during the non-connected tests
start_openocd_DAPLINK_2   
arm-none-eabi-gdb $TARGET_2_LC.elf -ex "target extended-remote localhost:3334"  -ex "monitor reset halt" -ex disconnect -ex quit
echo "> --- Halted helper MCU"

# tests projects
for dir in ./*/; do
    #(cd "$dir")
    if [[ "$dir" == *"$project_filter"* ]]; then
    
        export PROJECT_NAME=$(echo "$dir" | tr -d /.) 
        case $PROJECT_NAME in

        "BLE_datc")
            run_notConntectedTest ./ble_datc_test  "/dev/$devSerial_1" $VERBOSE_TEST
            ;;

        "BLE_dats")
            run_notConntectedTest ./ble_dats_test  "/dev/$devSerial_1" $VERBOSE_TEST
            ;;

        "BLE_mcs" )
            run_notConntectedTest ./ble_mcs_test  "/dev/$devSerial_1" $VERBOSE_TEST
            ;;

        "BLE_fit" )
            run_notConntectedTest ./ble_fit_test  "/dev/$devSerial_1" $VERBOSE_TEST
            ;;

        # "BLE_fcc" )
        #     # todo:
        #     # execute related test
        #     echo Found BLE_fcc #place holder
        #     ;;

        "BLE_FreeRTOS" )
            run_notConntectedTest ./ble_freeRTOS_test  "/dev/$devSerial_1" $VERBOSE_TEST
            ;;

        "BLE_otac" )
            run_notConntectedTest ./ble_otac_test  "/dev/$devSerial_1" $VERBOSE_TEST 
            ;;

        "BLE_otas" )
            # gets tested during conencted test below
            echo "> -----Found BLE_otas" #place holder
            ;;
            
        "BLE_periph" )
            # No buttons implemented for this example
            cd $PROJECT_NAME
            make -j8
            let "numOfFailedTests+=$?"  
            cd $MSDK_DIR/Examples/$TARGET_UC          
             ;;

        *)
            
            ;;

        esac

    fi
    let projIdx++

done

# Start connected-state tests
printf "\r\n ━━━━━━━━━━━━━━━━━━━━━━━|BLE_datc/dats Connection Test|━━━━━━━━━━━━━━━━━━━━━━━ \r\n\r\n"
#------ datc ME17
cd $MSDK_DIR/Examples/$TARGET_1_UC/BLE_datc/build
echo "> Flashing BLE_datc"
#talks to CMSIS_DAP_ID_1
arm-none-eabi-gdb $TARGET_1_LC.elf -ex "target extended-remote localhost:3333"  -ex "monitor reset halt" -ex load  -ex "monitor reset run" -ex disconnect -ex quit

#------ dats on ME18
cd $MSDK_DIR/Examples/$TARGET_2_UC/BLE_dats
make -j8
cd $MSDK_DIR/Examples/$TARGET_2_UC/BLE_dats/build
echo "> Flashing BLE_dats"    
arm-none-eabi-gdb $TARGET_2_LC.elf -ex "target extended-remote localhost:3334"  -ex "monitor reset halt" -ex load  -ex "monitor reset run" -ex disconnect -ex quit

#give them time to connect
sleep 4 #give them time to connect
cd $MSDK_DIR/Examples_tests
#                     server        client
./ble_datcs_test  "/dev/$devSerial_2" "/dev/$devSerial_1" $VERBOSE_TEST
let "numOfFailedTests+=$?" 

printf "\r\n ━━━━━━━━━━━━━━━━━━━━━━━|BLE_otac/otas Connection Test|━━━━━━━━━━━━━━━━━━━━━━━ \r\n\r\n"
#------ otac ME17
cd $MSDK_DIR/Examples/$TARGET_1_UC/BLE_otac/build
echo "> Flashing BLE_otac"
#talks to CMSIS_DAP_ID_1
arm-none-eabi-gdb $TARGET_1_LC.elf -ex "target extended-remote localhost:3333"  -ex "monitor reset halt" -ex load  -ex "monitor reset run" -ex disconnect -ex quit
kill -9 $openocd_dapLink1_pid

#------ otas on ME18
cd $MSDK_DIR/Examples/$TARGET_2_UC/BLE_otas
make -j8
cd $MSDK_DIR/Examples/$TARGET_2_UC/BLE_otas/build
echo "> Flashing BLE_otas"    
arm-none-eabi-gdb $TARGET_2_LC.elf -ex "target extended-remote localhost:3334"  -ex "monitor reset halt" -ex load -ex 'set $pc = 0x10020a80'  -ex "continue" -ex disconnect -ex quit & armGDBpid=$!

# Give gdb time to flash & client/server connection to be established
sleep 30
#have to kill to end gdb "continue" command
kill -9 $armGDBpid

cd $MSDK_DIR/Examples_tests
#                     server        cleint
./ble_otacs_test  "/dev/$devSerial_2" "/dev/$devSerial_1" $VERBOSE_TEST  
let "numOfFailedTests+=$?" 

echo "------------------------------------------------------------------------"
echo "          >>>>>> Test completed with $numOfFailedTests failed tests<<<<<<<"
echo "-----------------------------------------------------------------------"
exit $numOfFailedTests