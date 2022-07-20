#!/bin/bash
## Utility to build Qt for Android, on Linux
# References:
# - https://wiki.qt.io/Building_Qt_6_from_Git#Getting_the_source_code
# - https://doc.qt.io/qt-6/android-building.html
# - https://doc.qt.io/qt-6/android-getting-started.html

## WHY:
# Qt Android QtWebView does not have any way of allowing Camera/Mic permissions in a webpage, apparently
# Due to this bug: https://bugreports.qt.io/browse/QTBUG-63731
# So we need to hack and rebuild Android for Qt (at least some):
    # Hack in file: QtAndroidWebViewController.java
    # - Add following function to inner class QtAndroidWebChromeClient:
    #     @Override public void onPermissionRequest(PermissionRequest request) { request.grant(request.getResources()); }
    # - copy built jars QtAndroidWebView.jar, QtAndroidWebView-bundled.jar to Qt installation to rebuild

## REQUIREMENTS (provided by Github ubuntu 2004 build image):
# - gradle 7.2+
# - android cli tools (sdkmanager)
# - cmake 

## If building locally:
    # Install Android sdk cli tools from DL from https://developer.android.com/studio : commandlinetools-linux-8512546_latest.zip
    # - install to /home/user/Android/cmdline-tools ( SDK_ROOT = /home/user/Android )
    # Install Gradle 7.2+ from https://gradle.org/next-steps/?version=7.5&format=bin
    # - install to /opt/gradle
    # Do SDK setup as per:
    #  - ./sdkmanager --sdk_root=/home/user/Android --install "cmdline-tools;latest" "platform-tools" "platforms;android-31" "build-tools;31.0.0" "ndk;22.1.7171670"
    ## env vars:
    # export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
    # export CMAKE_ROOT=/home/dan/Qt/Tools/CMake/bin
    # export GRADLE_HOME=/opt/gradle
    # export PATH=$JAVA_HOME/bin:$GRADLE_HOME/bin:$CMAKE_ROOT:$PATH
    ## Build Qt
    # mkdir /home/user/code/qt5/qt6-build
    # cd /home/user/code/qt5/qt6-build
    # # actually!! to avoid errors, change to non-symlinked path ie /mnt/c/ ... on WSL
    # ../qt5/configure -platform android-clang -prefix /home/user/code/qt6_nu_install_arm7 \
    #     -android-ndk /home/user/Android/ndk/22.1.7171670/ -android-sdk  /home/user/Android/ \
    #     -qt-host-path /home/user/Qt/6.3.0/gcc_64 -android-abis arm64-v8a
    # cmake --build . --parallel

    # ../qt5/configure -platform android-clang -prefix /home/dan/code/qt6_nu_install_arm7 \
    #     -android-ndk /home/dan/Android/ndk/22.1.7171670/ -android-sdk  /home/dan/Android/ \
    #     -qt-host-path /home/dan/Qt/6.3.0/gcc_64 -android-abis armeabi-v7a

## AUTOBUILD:
# Install build deps from apt
sudo apt install -y --no-recommends \
    openjdk-11-jdk \
    ninja-build \
    flex bison \
    libgl-dev \
    libclang-11-dev \
    gperf \
    nodejs

# Python deps for build
sudo pip install html5lib
sudo pip install aqtinstall

# Install Qt 
mkdir $HOME/Qt
cd $HOME/Qt
aqt install-qt linux desktop 6.3.0 -m qtshadertools

# Set path env vars for build
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
export PATH=$JAVA_HOME/bin:$PATH

# Get Qt source - only base and modules necessary for QtWebView build
# NOTE: "qt5" is legacy name of repo in qt.io, but the build is qt6 !
cd $HOME
# 1) From git...
git clone git://code.qt.io/qt/qt5.git  # maybe add:  --depth 1 --shallow-submodules --no-single-branch
cd qt5
git checkout 6.3.0
perl init-repository --module-subset=qtbase,qtwebview,qtshadertools,qtdeclarative # get submodule source code

# Copy hacked source file into place, overwrite original
cp koord-realtime/android/qt_build_fix/QtAndroidWebViewController.java \
    qtwebview/src/jar/src/org/qtproject/qt/android/view/QtAndroidWebViewController.java

## OR 2) from qt archives
# mkdir qt5
# cd qt5
# # unpacks each archive to eg ./qtbase-everywhere-src-6.3.0/
# wget https://download.qt.io/archive/qt/6.3/6.3.0/submodules/qtbase-everywhere-src-6.3.0.tar.xz -qO- | tar xvJ
# wget https://download.qt.io/archive/qt/6.3/6.3.0/submodules/qtdeclarative-everywhere-src-6.3.0.tar.xz -qO- | tar xvJ
# wget https://download.qt.io/archive/qt/6.3/6.3.0/submodules/qtshadertools-everywhere-src-6.3.0.tar.xz -qO- | tar xvJ
# wget https://download.qt.io/archive/qt/6.3/6.3.0/submodules/qtwebview-everywhere-src-6.3.0.tar.xz -qO- | tar xvJ
# mv qtbase-everywhere-src-6.3.0 qtbase
# mv qtdeclarative-everywhere-src-6.3.0 qtdeclarative
# mv qtshadertools-everywhere-src-6.3.0 qtshadertools
# mv qtwebview-everywhere-src-6.3.0 qtwebview
# # Copy in CMakeLists, configure script
# cp linux/qt_build_fix/CMakeLists.txt CMakeLists.txt
# cp linux/qt_build_fix/configure configure
# cp -a linux/qt_build_fix/cmake cmake
# chmod 755 configure # ensure exec permission

# Create shadow build directory
mkdir -p $HOME/qt6-build

# Configure build for Android
# ALSO configure and build for: armeabi-v7a
cd $HOME/qt6-build
../qt5/configure \
    -platform android-clang \
    -prefix $HOME/qt6_nu_install \
    -android-ndk ${ANDROID_NDK_HOME} \
    -android-sdk ${ANDROID_SDK_ROOT} \
    -qt-host-path $HOME/Qt/6.3.0/gcc_64 \
    -android-abis arm64-v8a

# Build Qt for Android
cmake --build . --parallel

# Archive resultant jar here:
ls -al ./qtbase/jar/QtAndroidWebView.jar # <-- file to substitute

## Optional install to prefix dir
# cmake --install .
# file is now at $HOME/qt6_nu_install/jar/QtAndroidWebView.jar
