#!/bin/bash
# $ source test.sh
# $ ./arduino&
PROJECT="usbkwa"
IDEVER="1.8.19"
machine=`uname -m`
if [[ $machine =~ .*armv.* ]] ; then
    WORKDIR="/var/tmp/autobuild_usbkwa_$$"
    ARCH="linuxarm"
elif [[ $machine =~ .*aarch64.* ]] ; then
    WORKDIR="/var/tmp/autobuild_usbkwa_$$"
    ARCH="linuxaarch64"
else
    WORKDIR="/tmp/autobuild_usbkwa_$$"
    ARCH="linux64"
fi
mkdir -p ${WORKDIR}
# Install Ardino IDE in work directory
if [ ! -f ~/Downloads/arduino-${IDEVER}-${ARCH}.tar.xz ]
then
    wget https://downloads.arduino.cc/arduino-${IDEVER}-${ARCH}.tar.xz
    mv arduino-${IDEVER}-${ARCH}.tar.xz ~/Downloads
fi
tar xf ~/Downloads/arduino-${IDEVER}-${ARCH}.tar.xz -C ${WORKDIR}
# Create portable sketchbook and library directories
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
export PATH="${IDEDIR}:${PATH}"
cd ${IDEDIR}
which arduino
unzip -d ./tools ~/Downloads/EspExceptionDecoder-2.0.2.zip
if [ -d ~/Sync/ard_staging ]
then
    ln -s ~/Sync/ard_staging ${IDEDIR}/portable/staging
fi
arduino --pref "compiler.warning_level=default" \
    --pref "update.check=false" \
    --pref "editor.external=true" \
    --pref "custom_CDCOnBoot=esp32s2_cdc" \
    --save-prefs
arduino --pref "boardsmanager.additional.urls=https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json" --save-prefs
arduino --install-boards "esp32:esp32"
BOARD="esp32:esp32:adafruit_qtpy_esp32s3_nopsram"
arduino --board "${BOARD}" --save-prefs
CC="arduino --verify --board ${BOARD}"
arduino --install-library "WebSockets"
arduino --install-library "ArduinoJson"
arduino --install-library "WiFiManager"
ln -s ~/Sync/usbkwa ${LIBDIR}/..
cd ${IDEDIR}
ctags -R . ~/Sync/esp-idf
