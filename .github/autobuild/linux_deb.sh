#!/bin/bash
set -eu

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

    setup_cross_compilation_apt_sources

    echo "Installing dependencies..."
    sudo apt-get -qq update
    sudo apt-get -qq --no-install-recommends -y install devscripts build-essential debhelper fakeroot libjack-jackd2-dev
        # qt6-base-dev \
        # qt6-base-dev-tools \
        # qt6-tools-dev-tools \
        # qt6-webengine-dev \
        # qt6-webview-dev

    echo "Installing Qt..."
    if [[ "${TARGET_ARCH}" == amd64 ]]; then
        sudo python3 -m pip install "aqtinstall==${AQTINSTALL_VERSION}"
        # icu needs explicit installation 
        # otherwise: "qmake: error while loading shared libraries: libicui18n.so.56: cannot open shared object file: No such file or directory"
        sudo aqt install-qt --outputdir "${QT_DIR}" linux desktop "${QT_VERSION}" \
            --archives qtbase qtdeclarative qttools qttranslations icu \
            --modules qtwebview qtwebengine qtwebchannels qtpositioning
    else 
        # current latest version is 6.2.4
        # attempt force install
        sudo apt -f --no-install-recommends -y install \
            qt6-base-dev \
            qt6-base-dev-tools \
            qt6-tools-dev-tools \
            qt6-webengine-dev \
            qt6-webview-dev
    fi

    setup_cross_compiler
}

setup_cross_compilation_apt_sources() {
    if [[ "${TARGET_ARCH}" == amd64 ]]; then
       return
    fi
    sudo dpkg --add-architecture "${TARGET_ARCH}"
    sed -rne "s|^deb.*/ ([^ -]+(-updates)?) main.*|deb [arch=${TARGET_ARCH}] http://ports.ubuntu.com/ubuntu-ports \1 main universe multiverse restricted|p" /etc/apt/sources.list | sudo dd of=/etc/apt/sources.list.d/"${TARGET_ARCH}".list
    # sudo sed -re 's/^deb /deb [arch=amd64,i386] /' -i /etc/apt/sources.list

    # EXPERIMENTAL: add debian bullseye backports to Ubuntu 20.04 to add Qt6 armhf build
    sudo bash -c 'echo "deb http://deb.debian.org/debian bullseye-backports main contrib non-free" >> /etc/apt/sources.list'
    gpg --keyserver keyserver.ubuntu.com --recv-keys 0E98404D386FA1D9
    gpg --keyserver keyserver.ubuntu.com --recv-keys 648ACFD622F3D138
    gpg --export 0E98404D386FA1D9 | sudo apt-key add -
    gpg --export 648ACFD622F3D138 | sudo apt-key add -

    sudo sed -re 's/^deb /deb [arch=amd64,i386] /' -i /etc/apt/sources.list

    echo "STATE OF APT SOURCES.LIST:"
    cat /etc/apt/sources.list
}

setup_cross_compiler() {
    if [[ "${TARGET_ARCH}" == amd64 ]]; then
        return
    fi
    local GCC_VERSION=11  # 7 is the default on 18.04, there is no reason not to update once 18.04 is out of support
    sudo apt install -qq -y --no-install-recommends "g++-${GCC_VERSION}-${ABI_NAME}" "qmake6:${TARGET_ARCH}" "qt6-base-dev:${TARGET_ARCH}" "libjack-jackd2-dev:${TARGET_ARCH}"
    sudo update-alternatives --install "/usr/bin/${ABI_NAME}-g++" g++ "/usr/bin/${ABI_NAME}-g++-${GCC_VERSION}" 10
    sudo update-alternatives --install "/usr/bin/${ABI_NAME}-gcc" gcc "/usr/bin/${ABI_NAME}-gcc-${GCC_VERSION}" 10

    if [[ "${TARGET_ARCH}" == armhf ]]; then
        # Ubuntu's Qt version only ships a profile for gnueabi, but not for gnueabihf. Therefore, build a custom one:
        sudo cp -R "/usr/lib/${ABI_NAME}/qt6/mkspecs/linux-arm-gnueabi-g++/" "/usr/lib/${ABI_NAME}/qt6/mkspecs/${ABI_NAME}-g++/"
        sudo sed -re 's/-gnueabi/-gnueabihf/' -i "/usr/lib/${ABI_NAME}/qt6/mkspecs/${ABI_NAME}-g++/qmake.conf"
    fi
}

build_app_as_deb() {
    TARGET_ARCH="${TARGET_ARCH}" ./linux/deploy_deb.sh
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
        build_app_as_deb
        ;;
    get-artifacts)
        pass_artifacts_to_job
        ;;
    *)
        echo "Unknown stage '${1:-}'"
        exit 1
esac
