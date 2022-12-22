for sysdevpath in $(find /sys/bus/usb/devices/usb*/ -name dev); do
    (
        syspath="${sysdevpath%/dev}"
        devname="$(udevadm info -q name -p $syspath)"
        [[ "$devname" == "bus/"* ]] && exit
        eval "$(udevadm info -q property --export -p $syspath)"
        [[ -z "$ID_SERIAL" ]] && exit
       # echo "/dev/$devname - $ID_SERIAL"
        textToStrip="ARM_DAPLink_CMSIS-DAP_"
        if [[ $ID_SERIAL == *$textToStrip* ]]; then

            #store below variables into respective exported vars
            echo the port is /dev/${devname/ACM/"USB"}
            echo The ID is  ${ID_SERIAL#$textToStrip}
            
        fi
    )
done
