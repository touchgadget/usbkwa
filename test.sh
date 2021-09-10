#!/bin/bash
IDEVER="1.8.16"
WORKDIR="/tmp/autobuild_esp32s2_usbkwa_$$"
mkdir -p ${WORKDIR}
# Install Ardino IDE in work directory
if [ -f ~/Downloads/arduino-${IDEVER}-linux64.tar.xz ]
then
    tar xf ~/Downloads/arduino-${IDEVER}-linux64.tar.xz -C ${WORKDIR}
else
    wget -O arduino.tar.xz https://downloads.arduino.cc/arduino-${IDEVER}-linux64.tar.xz
    tar xf arduino.tar.xz -C ${WORKDIR}
    rm arduino.tar.xz
fi
# Create portable sketchbook and library directories
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
export PATH="${IDEDIR}:${PATH}"
cd ${IDEDIR}
which arduino
# Install board package
if [ -d ~/Sync/ard_staging ]
then
    ln -s ~/Sync/ard_staging ${IDEDIR}/portable/staging
fi
arduino --pref "compiler.warning_level=default" \
    --pref "update.check=false" \
    --pref "editor.external=true" \
    --save-prefs
arduino --pref "boardsmanager.additional.urls=https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json" --save-prefs
arduino --install-boards "esp32:esp32"
BOARD="esp32:esp32:esp32s2"
arduino --board "${BOARD}" --save-prefs
CC="arduino --verify --board ${BOARD}"
arduino --install-library "WebSockets"
arduino --install-library "ArduinoJson"
git clone https://github.com/tzapu/WiFiManager ${LIBDIR}/WiFiManager
cd ${LIBDIR}/WiFiManager
git checkout esp32s2
ln -s ~/Sync/usbkwa ${LIBDIR}/..
cd ${IDEDIR}
