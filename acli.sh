#!/bin/bash
# $ source acli.sh
PROJECT="usbkwa"
ARDDIR=/tmp/acli_${PROJECT}_$$
export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
export ARDUINO_DIRECTORIES_DATA="${ARDDIR}/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="${HOME}/Sync/ard_staging"
export ARDUINO_DIRECTORIES_USER="${ARDDIR}/user"
export LIBDIR="${ARDUINO_DIRECTORIES_USER}/libraries"
mkdir -p ${LIBDIR}
arduino-cli core --no-color update-index
arduino-cli core --no-color install esp32:esp32
arduino-cli core --no-color list
arduino-cli lib --no-color update-index
arduino-cli lib --no-color install "WebSockets"
arduino-cli lib --no-color install "ArduinoJson"
arduino-cli lib --no-color install "WiFiManager"
arduino-cli lib --no-color list
#
BOARD="esp32:esp32:adafruit_qtpy_esp32s3_nopsram"
alias cc='arduino-cli compile --clean --fqbn esp32:esp32:adafruit_qtpy_esp32s3_nopsram'
alias up='arduino-cli --fqbn esp32:esp32:adafruit_qtpy_esp32s3_nopsram upload --port /dev/ttyUSB0'
ctags -R . ${ARDDIR}  ~/Sync/esp-idf/
# Compile all examples for all boards with USB
declare -A BOARDS=( [esp32:esp32:esp32s2]="" [esp32:esp32:esp32s3]="--board-options USBMode=default" [esp32:esp32:adafruit_qtpy_esp32s3_nopsram]="")
for board in "${!BOARDS[@]}" ; do
    export ARDUINO_BOARD_FQBN=${board}
    ARDUINO_BOARD_FQBN2=${ARDUINO_BOARD_FQBN//:/.}
    board_opts=${BOARDS[$board]}
    (find . -name '*.ino' -print0 | xargs -0 -n 1 arduino-cli compile --no-color --verbose --fqbn ${board} ${board_opts} --output-dir "${ARDUINO_BOARD_FQBN2}") >${ARDDIR}/ci_${board}.log 2>&1
done
grep -Ei "^Sketch uses|^Global variables use" ${ARDDIR}/ci_*.log
grep -Ei "^Error|abort|missing|fatal|error:|warning:" ${ARDDIR}/ci_*.log
