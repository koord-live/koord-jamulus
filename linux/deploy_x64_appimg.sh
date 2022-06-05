#!/bin/bash
set -eu

# Create AppImage files (gui client and headless)

TARGET_ARCH="${TARGET_ARCH:-amd64}"

# cp -r distributions/debian .

# get the koord-rt version from pro file
KOORD_VERSION=$(grep -oP '^VERSION = \K\w[^\s\\]*' Koord-RT.pro)

# set up QT path
# NOTE: need to PREPEND to the path, to avoid running into all the alias crap that qtchooser installs to /usr/bin, all broken with Qt6 / qmake
# note: move off qmake to cmake!
export PATH=/usr/lib/qt6/bin:${PATH}

echo "${KOORD_VERSION} building..."

# base dir for build operations
BDIR="$(echo ${PWD})"

# Install appimage-builder
sudo apt install -y python3-pip python3-setuptools patchelf desktop-file-utils libgdk-pixbuf2.0-dev fakeroot strace fuse
sudo pip3 install appimage-builder
sudo wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -O /usr/local/bin/appimagetool
sudo chmod u+x /usr/local/bin/appimagetool

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
appimage-builder --appdir ../appdir_gui --skip-test --recipe linux/AppImageBuilder.yml
mkdir gui_appimage
mv Koord-RT-*.AppImage gui_appimage/


# echo "Building headless AppImage ...."
# cd $BDIR

# 
cd $BDIR
appimage-builder --appdir ../appdir_headless --skip-test --recipe linux/AppImageBuilder.yml
mkdir headless_appimage
mv Koord-RT-*.AppImage headless_appimage/


# # manually copy in desktop file
# mkdir -p appdir_headless/usr/share/applications/
# cp -v linux/koordrt-headless.desktop appdir_headless/usr/share/applications/
# # manually copy in image
# mkdir -p appdir_headless/usr/share/icons/hicolor/512x512/apps/
# cp -v distributions/koordrt.png appdir_headless/usr/share/icons/hicolor/512x512/apps/


# ./linuxdeployqt-continuous-x86_64.AppImage appdir_headless/usr/share/applications/*.desktop -appimage
# mkdir headless_appimage
# mv Koord-RT-*.AppImage headless_appimage/
