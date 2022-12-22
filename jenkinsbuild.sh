export CMSIS_DAP_ID_1=04091702adca825d00000000000000000000000097969906
#export CMSIS_DAP_ID_2=0409170246dfc09500000000000000000000000097969906
export TARGET_LC=max32655
export TARGET_UC=MAX32655
export TARGET_CFG=${TARGET_LC}.cfg
export TEST_BOARD=EvKit_V1
export MSDK_DIR=$(pwd)
export APP_EXAMPLES_PATH=$MSDK_DIR/Examples
export BLE_DATC_READY=0
export BLE_DATS_READY=0
export EXAMPLE_TEST_PATH=$MSDK_DIR/Examples_tests

# Get a fresh working copy
git clean -f -x -d
git checkout *
git reset --hard

# Get onto the appropriate branch
set +e
git branch -D ${BRANCH}
git checkout -b ${BRANCH} origin/${BRANCH}
set -e

# Make sure we're up to date
git pull
git submodule init
git submodule update

# cd Examples/MAX32655
# SUBDIRS=$(find . -type d -name "*BLE_dat*")
# for dir in ${SUBDIRS}
# do
#     echo "--------------------------------------------------------------------------------------------------------------------------------------------"
# 	echo "                                               Validation build for ${dir}    	                                      "
#     echo "--------------------------------------------------------------------------------------------------------------------------------------------"
#     cd ${dir}
#     make clean
#     make -j8
# 	cd ../
# done
cd $MSDK_DIR
ls -l

# Temporary -
cp -rp /home/lparm/examplesTestResources/Examples_tests/ .
cp Examples_tests/examplesTest.sh .
#./examplesTest.sh
which openocd 


openocd -f tcl/interface/cmsis-dap.cfg -f tcl/target/max32655.cfg -s /home/lparm/Tools/openocd/tcl  -c "cmsis_dap_serial  ${CMSIS_DAP_ID_1}" -c "gdb_port 3333" -c "telnet_port 4444" -c "tcl_port 6666"  &

ls 

