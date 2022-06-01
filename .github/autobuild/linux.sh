#!/bin/bash
set -eu

# for x64 AppImage build
export QT_VERSION=6.3.0
export QT_DIR="/usr/local/opt/qt"
# export PATH="${PATH}:${QT_DIR}/${QT_VERSION}/gcc_64/bin/"
AQTINSTALL_VERSION=2.1.0

if [[ ! ${JAMULUS_BUILD_VERSION:-} =~ [0-9]+\.[0-9]+\.[0-9]+ ]]; then
    echo "Environment variable JAMULUS_BUILD_VERSION has to be set to a valid version string"
    exit 1
fi

TARGET_ARCH="${TARGET_ARCH:-amd64}"
case "${TARGET_ARCH}" in
    amd64)
        ABI_NAME=""
        ;;
    armhf)
        ABI_NAME="arm-linux-gnueabihf"
        ;;
    *)
        echo "Unsupported TARGET_ARCH ${TARGET_ARCH}"
        exit 1
esac

setup() {
    export DEBIAN_FRONTEND="noninteractive"

    if [[ "${TARGET_ARCH}" == amd64 ]]; then
        setup_x64
    else
        setup_arm
    fi
}

setup_x64() {
    # This is on Ubuntu 18.04 and Qt 5.15.x
    # Why? Because this: https://github.com/probonopd/linuxdeployqt/issues/340
    # AppImage policy to force compatibility with oldest-supported LTS - so 18.04 until 2023.
    # Maybe I can do aqt install to 18.04 though ???

    echo "Installing dependencies..."
    sudo apt-get update
    sudo apt-get --no-install-recommends -y install devscripts build-essential debhelper fakeroot libjack-jackd2-dev libgl1-mesa-dev

    echo "Installing Qt..."
    sudo python3 -m pip install "aqtinstall==${AQTINSTALL_VERSION}"
    # icu needs explicit installation 
    # otherwise: "qmake: error while loading shared libraries: libicui18n.so.56: cannot open shared object file: No such file or directory"
    sudo aqt install-qt --outputdir "${QT_DIR}" linux desktop "${QT_VERSION}" \
        --archives qtbase qtdeclarative qttools qttranslations icu \
        --modules qtwebview qtwebengine qtwebchannel qtpositioning
}

setup_arm() {
    # This is on Ubuntu 18.04 and Qt 5.15.x

    echo "Configuring dpkg architectures for cross-compilation ..."
    sudo dpkg --add-architecture "${TARGET_ARCH}"
    sed -rne "s|^deb.*/ ([^ -]+(-updates)?) main.*|deb [arch=${TARGET_ARCH}] http://ports.ubuntu.com/ubuntu-ports \1 main universe multiverse restricted|p" /etc/apt/sources.list | sudo dd of=/etc/apt/sources.list.d/"${TARGET_ARCH}".list
    sudo sed -re 's/^deb /deb [arch=amd64,i386] /' -i /etc/apt/sources.list

    echo "Installing dependencies..."
    sudo apt-get update
    sudo apt-get --no-install-recommends -y install devscripts build-essential debhelper fakeroot libjack-jackd2-dev libgl1-mesa-dev
 
    echo "Installing Qt ...."
    sudo apt-get --no-install-recommends -y install \
        qtbase5-dev \
        qtbase5-dev-tools \
        qtwebengine5-dev \
        qml-module-qtwebview

    echo "Setting up cross-compiler ...."
    local GCC_VERSION=7  # 7 is the default on 18.04, there is no reason not to update once 18.04 is out of support
    sudo apt install -qq -y --no-install-recommends "g++-${GCC_VERSION}-${ABI_NAME}" "qmake5:${TARGET_ARCH}" "qtbase5-dev:${TARGET_ARCH}" "libjack-jackd2-dev:${TARGET_ARCH}"
    sudo update-alternatives --install "/usr/bin/${ABI_NAME}-g++" g++ "/usr/bin/${ABI_NAME}-g++-${GCC_VERSION}" 10
    sudo update-alternatives --install "/usr/bin/${ABI_NAME}-gcc" gcc "/usr/bin/${ABI_NAME}-gcc-${GCC_VERSION}" 10

    if [[ "${TARGET_ARCH}" == armhf ]]; then
        # Ubuntu's Qt version only ships a profile for gnueabi, but not for gnueabihf. Therefore, build a custom one:
        sudo cp -R "/usr/lib/${ABI_NAME}/qt5/mkspecs/linux-arm-gnueabi-g++/" "/usr/lib/${ABI_NAME}/qt5/mkspecs/${ABI_NAME}-g++/"
        sudo sed -re 's/-gnueabi/-gnueabihf/' -i "/usr/lib/${ABI_NAME}/qt5/mkspecs/${ABI_NAME}-g++/qmake.conf"
    fi

}

build_app() {
    if [[ "${TARGET_ARCH}" == armhf ]]; then
        TARGET_ARCH="${TARGET_ARCH}" ./linux/deploy_arm.sh
    else
        TARGET_ARCH="${TARGET_ARCH}" ./linux/deploy_x64.sh
    fi
}

pass_artifacts_to_job() {
    mkdir deploy

    # rename headless first, so wildcard pattern matches only one file each
    local artifact_1="koord-rt_headless_${JAMULUS_BUILD_VERSION}_ubuntu_${TARGET_ARCH}.deb"
    echo "Moving headless build artifact to deploy/${artifact_1}"
    mv ../koord-rt-headless*"_${TARGET_ARCH}.deb" "./deploy/${artifact_1}"
    echo "::set-output name=artifact_1::${artifact_1}"

    local artifact_2="koord-rt_${JAMULUS_BUILD_VERSION}_ubuntu_${TARGET_ARCH}.deb"
    echo "Moving regular build artifact to deploy/${artifact_2}"
    mv ../koord-rt*_"${TARGET_ARCH}.deb" "./deploy/${artifact_2}"
    echo "::set-output name=artifact_2::${artifact_2}"
}

case "${1:-}" in
    setup)
        setup
        ;;
    build)
        build_app
        ;;
    get-artifacts)
        pass_artifacts_to_job
        ;;
    *)
        echo "Unknown stage '${1:-}'"
        exit 1
esac
