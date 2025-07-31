#!/bin/bash

addremove=$1
model=$2
usbpath=$3

if [ $addremove == "add" ] && [ "$usbpath" != "" ]; then
    sleep 1
    echo $usbpath > /tmp/android_device
    echo $model >> /tmp/android_device
    echo "" > /dev/tty3
    echo "[${CYAN}${BOLD} INFO ${RESET}] *******************************************************" > /dev/tty3
    echo "[${CYAN}${BOLD} INFO ${RESET}] Device detected!" > /dev/tty3
    echo "[${CYAN}${BOLD} INFO ${RESET}] Model: $model" > /dev/tty3
    echo "[${CYAN}${BOLD} INFO ${RESET}] Path: $usbpath" > /dev/tty3
    echo "[${CYAN}${BOLD} INFO ${RESET}] *******************************************************" > /dev/tty3
    #log_echo "Device detected - $usbpath - $model"
fi

if [ "$addremove" == "remove" ] && [ "$usbpath" != "" ]; then
    if [ -f /tmp/android_device ]; then
        CHECK=$(cat /tmp/android_device | grep $usbpath)
        if [ ! -z $CHECK ]; then
            sudo rm /tmp/android_device
            echo "" > /dev/tty3
            echo "[${RED}${BOLD} WARN ${RESET}] *******************************************************" > /dev/tty3
            echo "[${RED}${BOLD} WARN ${RESET}] Device removed!" > /dev/tty3
            echo "[${RED}${BOLD} WARN ${RESET}] Model: $model" > /dev/tty3
            echo "[${RED}${BOLD} WARN ${RESET}] Path: $usbpath" > /dev/tty3
            echo "[${RED}${BOLD} WARN ${RESET}] *******************************************************" > /dev/tty3
            log_echo "Device removed - $usbpath - $model"
            sleep 1 # relax time for failsafe while android phone is switching mode
                    # while starting google auto
        fi
    fi
fi
