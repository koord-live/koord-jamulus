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
export PATH=/usr/lib/qt6/bin/:/usr/lib/qt6/libexec/:${PATH}

echo "${KOORD_VERSION} building..."

# base dir for build operations
BDIR="$(echo ${PWD})"

# install fuse2 as dep for appimage
sudo apt-get install -y libfuse2

# Install linuxdeploy
sudo wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O /usr/local/bin/linuxdeploy
sudo chmod 755 /usr/local/bin/linuxdeploy
sudo wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -O /usr/local/bin/linuxdeploy-plugin-qt
sudo chmod 755 /usr/local/bin/linuxdeploy-plugin-qt

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

echo "DEBUG:"
echo "ldd /usr/local/opt/qt/6.3.0/gcc_64/plugins/platforms/libqxcb.so ..."
ldd /usr/local/opt/qt/6.3.0/gcc_64/plugins/platforms/libqxcb.so 

echo "Building gui AppImage ...."
cd $BDIR
linuxdeploy -v0 --appdir appdir_gui --plugin qt --output Koord-RT-${VERSION}.appimage
mv Koord-RT-*.AppImage gui_appimage/

# 
echo "Building headless AppImage ...."
cd $BDIR
linuxdeploy -v0 --appdir appdir_headless --plugin qt --output Koord-RT_headless_${VERSION}.appimage
mv Koord-RT-*.AppImage headless_appimage/

