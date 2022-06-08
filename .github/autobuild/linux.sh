#!/bin/bash
set -eu

# for x64 AppImage build
export QT_VERSION=6.3.0
export QT_DIR="/usr/local/opt/qt"
export PATH="${PATH}:${QT_DIR}/${QT_VERSION}/gcc_64/bin/"
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

    # # VIA APT REPOS
    echo "Installing dependencies..."
    sudo apt-get update
    sudo apt-get --no-install-recommends -y install devscripts build-essential debhelper fakeroot libjack-jackd2-dev libgl-dev libegl1

    echo "Installing Qt..."
    sudo apt-get --no-install-recommends -y install \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-tools-dev-tools \
        qt6-declarative-dev \
        qt6-webengine-dev \
        qt6-webengine-dev-tools \
        qt6-webview-dev \
        qt6-webview-plugins \
        qml6-module-qtwebview \
        libqt6opengl6-dev \
        qt6-l10n-tools

    # # via AQTINSTALL
    # echo "Installing dependencies..."
    # sudo apt-get update
    # sudo apt-get --no-install-recommends -y install devscripts build-essential debhelper fakeroot libjack-jackd2-dev libgl-dev libegl1 \
    #     python3-setuptools python3-wheel

    # echo "Installing Qt..."
    # sudo pip3 install "aqtinstall==${AQTINSTALL_VERSION}"
    # sudo python3 -m aqt install-qt --outputdir "${QT_DIR}" linux desktop "${QT_VERSION}" \
    #     --archives qtbase qtdeclarative qttools qttranslations icu \
    #     --modules qtwebview qtwebengine qtwebchannel qtpositioning

}

setup_arm() {

    echo "Configuring dpkg architectures for cross-compilation ..."
    sudo dpkg --add-architecture "${TARGET_ARCH}"
    sed -rne "s|^deb.*/ ([^ -]+(-updates)?) main.*|deb [arch=${TARGET_ARCH}] http://ports.ubuntu.com/ubuntu-ports \1 main universe multiverse restricted|p" /etc/apt/sources.list | sudo dd of=/etc/apt/sources.list.d/"${TARGET_ARCH}".list
    sudo sed -re 's/^deb /deb [arch=amd64,i386] /' -i /etc/apt/sources.list

    echo "Installing dependencies..."
    sudo apt-get update
    sudo apt-get --no-install-recommends -y install devscripts build-essential debhelper fakeroot libjack-jackd2-dev libgl-dev libegl1
 
    echo "Installing Qt ...."
    sudo apt-get --no-install-recommends -y install \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-tools-dev-tools \
        qt6-declarative-dev \
        qt6-webengine-dev \
        qt6-webengine-dev-tools \
        qt6-webview-dev \
        qt6-webview-plugins \
        qml6-module-qtwebview \
        libqt6opengl6-dev \
        qt6-l10n-tools

    echo "Setting up cross-compiler ...."
    local GCC_VERSION=11
    sudo apt-get install -y --no-install-recommends \
        "g++-${GCC_VERSION}-${ABI_NAME}" \
        "libjack-jackd2-dev:${TARGET_ARCH}" \
        "libgl-dev:${TARGET_ARCH}" \
        "libegl1:${TARGET_ARCH}" \
        "qmake6:${TARGET_ARCH}" \
        "qt6-base-dev:${TARGET_ARCH}" \
        "qt6-declarative-dev:${TARGET_ARCH}" \
        "qt6-webengine-dev:${TARGET_ARCH}" \
        "qt6-webengine-dev-tools:${TARGET_ARCH}" \
        "qt6-webview-dev:${TARGET_ARCH}" \
        "qt6-webview-plugins:${TARGET_ARCH}" \
        "qml6-module-qtwebview:${TARGET_ARCH}" \
        "libqt6opengl6-dev:${TARGET_ARCH}"

    sudo update-alternatives --install "/usr/bin/${ABI_NAME}-g++" g++ "/usr/bin/${ABI_NAME}-g++-${GCC_VERSION}" 10
    sudo update-alternatives --install "/usr/bin/${ABI_NAME}-gcc" gcc "/usr/bin/${ABI_NAME}-gcc-${GCC_VERSION}" 10

    if [[ "${TARGET_ARCH}" == armhf ]]; then
        # Ubuntu's Qt version only ships a profile for gnueabi, but not for gnueabihf. Therefore, build a custom one:
        sudo cp -R "/usr/lib/${ABI_NAME}/qt6/mkspecs/linux-arm-gnueabi-g++/" "/usr/lib/${ABI_NAME}/qt6/mkspecs/${ABI_NAME}-g++/"
        sudo sed -re 's/-gnueabi/-gnueabihf/' -i "/usr/lib/${ABI_NAME}/qt6/mkspecs/${ABI_NAME}-g++/qmake.conf"
    fi

    # force link for uic
    sudo ln -s /usr/lib/qt6/libexec/* /usr/libexec/

}

build_app() {
    if [[ "${TARGET_ARCH}" == armhf ]]; then
        TARGET_ARCH="${TARGET_ARCH}" ./linux/deploy_deb.sh
    else
        TARGET_ARCH="${TARGET_ARCH}" ./linux/deploy_x64_appimg.sh
        # TARGET_ARCH="${TARGET_ARCH}" ./linux/deploy_deb.sh
    fi
}

pass_artifacts_to_job() {
    mkdir deploy

    if [[ "${TARGET_ARCH}" == armhf ]]; then
        # rename headless first, so wildcard pattern matches only one file each
        local artifact_1="koord-rt_headless_${JAMULUS_BUILD_VERSION}_ubuntu_${TARGET_ARCH}.deb"
        echo "Moving headless build artifact to deploy/${artifact_1}"
        mv ../koord-rt-headless*"_${TARGET_ARCH}.deb" "./deploy/${artifact_1}"
        echo "::set-output name=artifact_1::${artifact_1}"

        local artifact_2="koord-rt_${JAMULUS_BUILD_VERSION}_ubuntu_${TARGET_ARCH}.deb"
        echo "Moving regular build artifact to deploy/${artifact_2}"
        mv ../koord-rt*_"${TARGET_ARCH}.deb" "./deploy/${artifact_2}"
        echo "::set-output name=artifact_2::${artifact_2}"
    else
        # rename headless first, so wildcard pattern matches only one file each
        local artifact_1="koord-rt_headless_${JAMULUS_BUILD_VERSION}_${TARGET_ARCH}.AppImage"
        echo "Moving headless build artifact to deploy/${artifact_1}"
        mv headless_appimage/*appimage "./deploy/${artifact_1}"
        echo "::set-output name=artifact_1::${artifact_1}"

        local artifact_2="koord-rt_${JAMULUS_BUILD_VERSION}_${TARGET_ARCH}.AppImage"
        echo "Moving regular build artifact to deploy/${artifact_2}"
        mv gui_appimage/*appimage "./deploy/${artifact_2}"
        echo "::set-output name=artifact_2::${artifact_2}"
    fi
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
