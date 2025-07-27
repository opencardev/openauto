#!/bin/bash

# OpenAuto with comprehensive logging
LOG_FILE="/var/log/openauto/openauto.log"

echo "=== OpenAuto Debug Session Started at $(date) ===" | tee -a "$LOG_FILE"

# Set debug environment
export OPENAUTO_LOG_LEVEL=DEBUG
export OPENAUTO_DEBUG_MODE=1
export BOOST_LOG_TRIVIAL_VERBOSITY=debug
export QT_QPA_PLATFORM=linuxfb

# Start autoapp with output to both console and file
./autoapp 2>&1 | tee -a "$LOG_FILE"

echo "=== OpenAuto Debug Session Ended at $(date) ===" | tee -a "$LOG_FILE"
