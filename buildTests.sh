# TODO: preferably a make file
gcc -ggdb -o ble_datc_test testSources/ble_datc_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h 
gcc -ggdb -o ble_dats_test testSources/ble_dats_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h 
gcc -ggdb -o ble_mcs_test testSources/ble_mcs_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h 
gcc -ggdb -o ble_otac_test testSources/ble_otac_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h 
gcc -ggdb -o ble_datcs_test testSources/ble_datcs_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h 
gcc -ggdb -o ble_fit_test testSources/ble_fit_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h
gcc -ggdb -o ble_otacs_test testSources/ble_otacs_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h
gcc -ggdb -o ble_freeRTOS_test testSources/ble_freeRTOS_test.c serialTestLib/serialTest.c serialTestLib/serialTest.h
