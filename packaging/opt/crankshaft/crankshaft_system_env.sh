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


if [ -f /boot/crankshaft/crankshaft_env.sh ]; then
    source /boot/crankshaft/crankshaft_env.sh
fi

# colors
RED=`tput setaf 1 -T xterm`
GREEN=`tput setaf 2 -T xterm`
YELLOW=`tput setaf 3 -T xterm`
CYAN=`tput setaf 6 -T xterm`
BLUE=`tput setaf 4 -T xterm`
MAGENTA=`tput setaf 5 -T xterm`
RESET=`tput sgr0 -T xterm && tput setaf 7 -T xterm`
BOLD=`tput bold -T xterm`

# set gpio default pin levels if global activated
if [ ! -z $ENABLE_GPIO ]; then
    if [ $ENABLE_GPIO -eq 1 ]; then
        if [ $DEV_PIN -ne 0 ]; then
            sudo /usr/bin/gpio -g mode $DEV_PIN up
        fi
        if [ $INVERT_PIN -ne 0 ]; then
            sudo /usr/bin/gpio -g mode $INVERT_PIN up
        fi
        if [ $X11_PIN -ne 0 ]; then
            sudo /usr/bin/gpio -g mode $X11_PIN up
        fi
    else
        # make sure flag is correctly set if
        # ENABLE_GPIO is not set to 1 or missing
        # to prevent from errors
        ENABLE_GPIO=0
    fi
fi

# set predefined state for standalone gpio's
if [ $REARCAM_PIN -ne 0 ]; then
    sudo /usr/bin/gpio -g mode $REARCAM_PIN up
fi

if [ $IGNITION_PIN -ne 0 ]; then
    sudo /usr/bin/gpio -g mode $IGNITION_PIN up
fi

if [ $DAYNIGHT_PIN -ne 0 ]; then
    sudo /usr/bin/gpio -g mode $DAYNIGHT_PIN up
fi

# callable functions
show_clear_screen() {
    plymouth --hide-splash > /dev/null 2>&1 # hide the boot splash
    #sleep 1
    plymouth --hide-splash > /dev/null 2>&1 # hide the boot splash
    chvt 3
    printf "\033[2J" > /dev/tty3 # clear screen
    printf "\033[0;0H\n" > /dev/tty3 # Move to 0,0
    log_echo "Show Clear Screen"
}

show_screen() {
    plymouth --hide-splash > /dev/null 2>&1 # hide the boot splash
    #sleep 1
    plymouth --hide-splash > /dev/null 2>&1 # hide the boot splash
    chvt 3
    log_echo "Show Screen"
}

show_cursor() {
    setterm -cursor on > /dev/tty3
    setterm -blink on > /dev/tty3
    log_echo "Show Cursor"
}

hide_cursor() {
    setterm -cursor off > /dev/tty3
    setterm -blink off > /dev/tty3
    log_echo "Hide Cursor"
}

log_echo() {
    logger "SERV-LOGGER: --------------------------------------------------------------------"
    logger "SERV-LOGGER: Caller: $0"
    logger "SERV-LOGGER: $1"
    logger "SERV-LOGGER: --------------------------------------------------------------------"
}
