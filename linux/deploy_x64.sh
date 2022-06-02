#!/bin/bash
set -eu

# Create AppImage files (gui client and headless)

TARGET_ARCH="${TARGET_ARCH:-amd64}"

# cp -r distributions/debian .

# get the koord-rt version from pro file
KOORD_VERSION=$(grep -oP '^VERSION = \K\w[^\s\\]*' Koord-RT.pro)

echo "${KOORD_VERSION} building..."

# base dir for build operations
BDIR="$(echo ${PWD})"

# Install linuxdeployqt
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage

# Configure ###################
# gui
echo "Configuring gui ...."
cd $BDIR
mkdir -p build-gui
cd build-gui
qmake "CONFIG+=noupcasename" PREFIX=/usr ../Koord-RT.pro

# headless
echo "Configuring headless ...."
cd $BDIR
mkdir -p build-nox
cd build-nox
qmake "CONFIG+=headless serveronly" TARGET=koord-rt-headless PREFIX=/usr ../Koord-RT.pro


# Build ############################
cd $BDIR
cd src/translation
lrelease *.ts

# gui
echo "Building gui ...."
cd $BDIR
cd build-gui
make -j "$(nproc)"

# headless
echo "Building headless ...."
cd $BDIR
cd build-nox
make -j "$(nproc)"

# Install ###########################
# gui
echo "Installing gui ...."
cd $BDIR
cd build-gui
make install INSTALL_ROOT=../appdir_gui
find ../appdir_gui

# headless
echo "Installing headless...."
cd $BDIR
cd build-nox
make install INSTALL_ROOT=../appdir_headless
find ../appdir_headless

# Build AppImage ########################
export VERSION=${KOORD_VERSION}

echo "Building gui AppImage ...."
cd $BDIR
./linuxdeployqt-continuous-x86_64.AppImage appdir_gui/usr/share/applications/*.desktop -appimage
mkdir gui_appimage
mv Koord-RT-*.AppImage gui_appimage/

echo "Building headless AppImage ...."
cd $BDIR
# manually copy in desktop file
mkdir -p appdir_headless/usr/share/applications/
cp -v linux/koordrt-headless.desktop appdir_headless/usr/share/applications/
# manually copy in image
mkdir -p appdir_headless/usr/share/icons/hicolor/512x512/apps/
cp -v distributions/koordrt.png appdir_headless/usr/share/icons/hicolor/512x512/apps/
./linuxdeployqt-continuous-x86_64.AppImage appdir_headless/usr/share/applications/*.desktop -appimage
mkdir headless_appimage
mv Koord-RT-*.AppImage headless_appimage/
