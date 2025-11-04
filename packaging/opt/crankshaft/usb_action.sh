#!/bin/bash
# * Project: OpenAuto
# * This file is part of openauto project.
# * Copyright (C) 2025 OpenCarDev Team
# *
# *  openauto is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  openauto is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with openauto. If not, see <http://www.gnu.org/licenses/>.

# USB action script for Crankshaft
# Handles addition and removal of Android devices

# Load Crankshaft environment variables
source /opt/crankshaft/bin/crankshaft_env.sh
source /opt/crankshaft/bin/log_functions.sh

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
    log_echo "Device detected - $usbpath - $model"
    /usr/local/bin/crankshaft timers stop
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
            if [ ! -f /tmp/dev_mode_enabled ] && [ ! -f /tmp/android_device ] && [ ! -f /tmp/aa_device ]; then
                log_echo "Start timers"
                /usr/local/bin/crankshaft timers start
            fi
        fi
    fi
fi
