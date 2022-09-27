    #!/bin/bash
set -eu

COMMANDLINETOOLS_VERSION=8512546
ANDROID_NDK_VERSION=r22b
ANDROID_PLATFORM=android-31
ANDROID_BUILD_TOOLS=31.0.0
AQTINSTALL_VERSION=2.1.0
QT_VERSION=6.3.2

# Only variables which are really needed by sub-commands are exported.
# Definitions have to stay in a specific order due to dependencies.
QT_BASEDIR="/opt/Qt"
ANDROID_BASEDIR="/opt/android"
BUILD_DIR=build
export ANDROID_SDK_ROOT="${ANDROID_BASEDIR}/android-sdk"
COMMANDLINETOOLS_DIR="${ANDROID_SDK_ROOT}"/cmdline-tools/latest/
export ANDROID_NDK_ROOT="${ANDROID_BASEDIR}/android-ndk"
# # WARNING: Support for ANDROID_NDK_HOME is deprecated and will be removed in the future. Use android.ndkVersion in build.gradle instead.
# # ref: https://bugreports.qt.io/browse/QTBUG-81978?focusedCommentId=497578&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-497578
# ANDROID_NDK_HOME=$ANDROID_NDK_ROOT
ANDROID_NDK_HOST="linux-x86_64"
ANDROID_SDKMANAGER="${COMMANDLINETOOLS_DIR}/bin/sdkmanager"
export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
export PATH="${PATH}:${ANDROID_SDK_ROOT}/tools"
export PATH="${PATH}:${ANDROID_SDK_ROOT}/platform-tools"

if [[ ! ${JAMULUS_BUILD_VERSION:-} =~ [0-9]+\.[0-9]+\.[0-9]+ ]]; then
    echo "Environment variable JAMULUS_BUILD_VERSION has to be set to a valid version string"
    exit 1
fi

setup_ubuntu_dependencies() {
    export DEBIAN_FRONTEND="noninteractive"

    sudo apt-get -qq update
    sudo apt-get -qq --no-install-recommends -y install \
        build-essential zip unzip bzip2 p7zip-full curl chrpath openjdk-11-jdk-headless
}

setup_android_sdk() {
    mkdir -p "${ANDROID_BASEDIR}"

    if [[ -d "${COMMANDLINETOOLS_DIR}" ]]; then
        echo "Using commandlinetools installation from previous run (actions/cache)"
    else
        mkdir -p "${COMMANDLINETOOLS_DIR}"
        curl -s -o downloadfile "https://dl.google.com/android/repository/commandlinetools-linux-${COMMANDLINETOOLS_VERSION}_latest.zip"
        unzip -q downloadfile
        mv cmdline-tools/* "${COMMANDLINETOOLS_DIR}"
    fi

    yes | "${ANDROID_SDKMANAGER}" --licenses
    "${ANDROID_SDKMANAGER}" --update
    "${ANDROID_SDKMANAGER}" "platforms;${ANDROID_PLATFORM}"
    "${ANDROID_SDKMANAGER}" "build-tools;${ANDROID_BUILD_TOOLS}"
}

setup_android_ndk() {
    mkdir -p "${ANDROID_BASEDIR}"

    if [[ -d "${ANDROID_NDK_ROOT}" ]]; then
        echo "Using NDK installation from previous run (actions/cache)"
    else
        echo "Installing NDK from dl.google.com to ${ANDROID_NDK_ROOT}..."
        curl -s -o downloadfile "https://dl.google.com/android/repository/android-ndk-${ANDROID_NDK_VERSION}-linux-x86_64.zip"
        unzip -q downloadfile
        mv "android-ndk-${ANDROID_NDK_VERSION}" "${ANDROID_NDK_ROOT}"
    fi
}

setup_qt() {
    if [[ -d "${QT_BASEDIR}" ]]; then
        echo "Using Qt installation from previous run (actions/cache)"
    else
        echo "Installing Qt..."
        python3 -m pip install "aqtinstall==${AQTINSTALL_VERSION}"
        # icu needs explicit installation 
        # otherwise: "qmake: error while loading shared libraries: libicui18n.so.56: cannot open shared object file: No such file or directory"
        python3 -m aqt install-qt --outputdir "${QT_BASEDIR}" linux desktop "${QT_VERSION}" \
            --archives qtbase qtdeclarative qtsvg qttools icu

        # - 64bit required for Play Store
        python3 -m aqt install-qt --outputdir "${QT_BASEDIR}" linux android "${QT_VERSION}" android_arm64_v8a \
            --archives qtbase qtdeclarative qtsvg qttools \
            --modules qtwebview 
        ##FIXME - HACK - SUBSTITUTE webview jar
        # wget https://github.com/koord-live/koord-app/releases/download/${QT_VERSION}/QtAndroidWebView_arm64-v8a.jar -O \
        wget https://github.com/koord-live/koord-app/releases/download/%24%7BQT_VERSION%7D/QtAndroidWebView_arm64-v8a.jar -O \
            "${QT_BASEDIR}/${QT_VERSION}/android_arm64_v8a/jar/QtAndroidWebView.jar"

        # Also install for arm_v7 to build for 32bit devices
        python3 -m aqt install-qt --outputdir "${QT_BASEDIR}" linux android "${QT_VERSION}" android_armv7 \
            --archives qtbase qtdeclarative qtsvg qttools \
            --modules qtwebview 
        ##FIXME - HACK - SUBSTITUTE webview jar
        # wget https://github.com/koord-live/koord-app/releases/download/${QT_VERSION}/QtAndroidWebView_armeabi-v7a.jar -O \
        wget https://github.com/koord-live/koord-app/releases/download/%24%7BQT_VERSION%7D/QtAndroidWebView_armeabi-v7a.jar -O \
            "${QT_BASEDIR}/${QT_VERSION}/android_armv7/jar/QtAndroidWebView.jar"

    fi
}

install_android_openssl() {
    echo ">> Installing android_openssl as build dep ..."
    wget https://github.com/KDAB/android_openssl/archive/refs/tags/1.1.1l_1.0.2u.tar.gz 
    mkdir android_openssl 
    tar zxvf 1.1.1l_1.0.2u.tar.gz -C android_openssl --strip-components 1
    # android openssl libs are now installed at ./android_openssl/
}

build_app() {
    local ARCH_ABI="${1}"

    local QT_DIR="${QT_BASEDIR}/${QT_VERSION}/android"
    local MAKE="${ANDROID_NDK_ROOT}/prebuilt/${ANDROID_NDK_HOST}/bin/make"

    echo "${GOOGLE_RELEASE_KEYSTORE}" | base64 --decode > android/android_release.keystore

    ##FIXME - temporary Oboe lib hackfix for Android12
    cp -v android/QuirksManager.cpp libs/oboe/src/common/QuirksManager.cpp

    echo ">>> Compiling for ${ARCH_ABI} ..."
    # if ARCH_ABI=android_armv7 we need to override ANDROID_ABIS for qmake 
    # note: seems ANDROID_ABIS can be set here at cmdline, but ANDROID_VERSION_CODE cannot - must be in qmake file
    if [ "${ARCH_ABI}" == "android_armv7" ]; then
        echo ">>> Running qmake with ANDROID_ABIS=armeabi-v7a ..."
        ANDROID_ABIS=armeabi-v7a \
            "${QT_BASEDIR}/${QT_VERSION}/${ARCH_ABI}/bin/qmake" -spec android-clang
    else
        echo ">>> Running qmake with ANDROID_ABIS=arm64-v8a ..."
        ANDROID_ABIS=arm64-v8a \
            "${QT_BASEDIR}/${QT_VERSION}/${ARCH_ABI}/bin/qmake" -spec android-clang
    fi
    "${MAKE}" -j "$(nproc)"
    "${MAKE}" INSTALL_ROOT="${BUILD_DIR}_${ARCH_ABI}" -f Makefile install
}

build_make_clean() {
    echo ">>> Doing make clean ..."
    local MAKE="${ANDROID_NDK_ROOT}/prebuilt/${ANDROID_NDK_HOST}/bin/make"
    "${MAKE}" clean
    rm -f Makefile
}

build_aab() {
    local ARCH_ABI="${1}"

    if [ "${ARCH_ABI}" == "android_armv7" ]; then
        TARGET_ABI=armeabi-v7a
    else
        TARGET_ABI=arm64-v8a
    fi
    echo ">>> Building .aab file for ${TARGET_ABI}...."

    ANDROID_ABIS=${TARGET_ABI} ${QT_BASEDIR}/${QT_VERSION}/gcc_64/bin/androiddeployqt --input android-Koord-deployment-settings.json \
        --verbose \
        --output "${BUILD_DIR}_${ARCH_ABI}" \
        --aab \
        --release \
        --sign android/android_release.keystore koord \
            --storepass ${GOOGLE_KEYSTORE_PASS} \
        --android-platform "${ANDROID_PLATFORM}" \
        --jdk "${JAVA_HOME}" \
        --gradle
}

pass_artifact_to_job() {
    local ARCH_ABI="${1}"
    echo ">>> Deploying .aab file for ${ARCH_ABI}...."

    if [ "${ARCH_ABI}" == "android_armv7" ]; then
        NUM="1"
        BUILDNAME="arm"
    else
        NUM="2"
        BUILDNAME="arm64"
    fi

    mkdir -p deploy
    local artifact="Koord_${JAMULUS_BUILD_VERSION}_android_${BUILDNAME}.aab"
    # debug to check for filenames
    ls -alR ${BUILD_DIR}_${ARCH_ABI}/build/outputs/bundle/release/
    ls -al ${BUILD_DIR}_${ARCH_ABI}/build/outputs/bundle/release/build_${ARCH_ABI}-release.aab
    echo ">>> Moving ${BUILD_DIR}_${ARCH_ABI}/build/outputs/bundle/release/build_${ARCH_ABI}-release.aab to deploy/${artifact}"
    mv "./${BUILD_DIR}_${ARCH_ABI}/build/outputs/bundle/release/build_${ARCH_ABI}-release.aab" "./deploy/${artifact}"
    echo ">>> Moved .aab file to deploy/${artifact}"
    echo ">>> Artifact number is: ${NUM}"
    echo ">>> Setting output as such: name=artifact_${NUM}::${artifact}"
    echo "::set-output name=artifact_${NUM}::${artifact}"
}

case "${1:-}" in
    setup)
        setup_ubuntu_dependencies
        setup_android_ndk
        setup_android_sdk
        setup_qt
        install_android_openssl
        ;;
    build)
        build_app "android_armv7"
        build_aab "android_armv7"
        build_make_clean
        build_app "android_arm64_v8a"
        build_aab "android_arm64_v8a"
        ;;
    get-artifacts)
        pass_artifact_to_job "android_armv7"
        pass_artifact_to_job "android_arm64_v8a"
        ;;
    *)
        echo "Unknown stage '${1:-}'"
        exit 1
esac
