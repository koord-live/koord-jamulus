#!/bin/bash
set -eu -o pipefail

root_path=$(pwd)
project_path="${root_path}/Koord.pro"
resources_path="${root_path}/src/res"
build_path="${root_path}/build"
deploy_path="${root_path}/deploy"
deploypkg_path="${root_path}/deploypkg"
macadhoc_cert_name=""
macapp_cert_name=""
macinst_cert_name=""
macos_pp=""

while getopts 'hs:a:i:' flag; do
    case "${flag}" in
        s)
            macadhoc_cert_name=$OPTARG
            if [[ -z "$macadhoc_cert_name" ]]; then
                echo "Please add the name of the adhoc certificate to use: -s \"<name>\""
            fi
            ;;
        a)
            macapp_cert_name=$OPTARG
            if [[ -z "$macapp_cert_name" ]]; then
                echo "Please add the name of the codesigning certificate to use: -a \"<name>\""
            fi
            ;;
        i)
            macinst_cert_name=$OPTARG
            if [[ -z "$macinst_cert_name" ]]; then
                echo "Please add the name of the installer signing certificate to use: -i \"<name>\""
            fi
            ;;
        h)
            echo "Usage: -s <adhoccertname> -a <codesigncertname> -i <instlrsigncertname> -p <macos_prov_profile>"
            exit 0
            ;;
        *)
            exit 1
            ;;
    esac
done

cleanup() {
    # Clean up previous deployments
    rm -rf "${build_path}"
    rm -rf "${deploy_path}"
    rm -rf "${deploypkg_path}"
    mkdir -p "${build_path}"
    mkdir -p "${deploy_path}"
    mkdir -p "${deploypkg_path}"
}

build_app_compile()
{
    # local client_or_server="${1}"

    # We need this in build environment otherwise defaults to webengine!!
    # bug is here: https://code.qt.io/cgit/qt/qtwebview.git/tree/src/webview/qwebviewfactory.cpp?h=6.3.1#n51
    # Note: not sure if this is useful here or only in Run env
    export QT_WEBVIEW_PLUGIN="native"

    # Build Jamulus
    declare -a BUILD_ARGS=("_UNUSED_DUMMY=''")  # old bash fails otherwise
    if [[ "${TARGET_ARCH:-}" ]]; then
        BUILD_ARGS=("QMAKE_APPLE_DEVICE_ARCHS=${TARGET_ARCH}" "QT_ARCH=${TARGET_ARCH}")
    fi
    qmake "${project_path}" -o "${build_path}/Makefile" "CONFIG+=release" "${BUILD_ARGS[@]}" "${@:2}"

    local target_name
    target_name=$(sed -nE 's/^QMAKE_TARGET *= *(.*)$/\1/p' "${build_path}/Makefile")
    
    local job_count
    job_count=$(sysctl -n hw.ncpu)

    # Get Jamulus version
    local app_version="$(cat "${project_path}" | sed -nE 's/^VERSION *= *(.*)$/\1/p')"

    make -f "${build_path}/Makefile" -C "${build_path}" -j "${job_count}"
}

build_app_compile_universal()
{
    # local client_or_server="${1}"

    # We need this in build environment otherwise defaults to webengine!!
    # bug is here: https://code.qt.io/cgit/qt/qtwebview.git/tree/src/webview/qwebviewfactory.cpp?h=6.3.1#n51
    # Note: not sure if this is useful here or only in Run env
    export QT_WEBVIEW_PLUGIN="native"

    local job_count
    job_count=$(sysctl -n hw.ncpu)

    # Build Jamulus for all requested architectures, defaulting to x86_64 if none provided:
    local target_name
    local target_arch
    local target_archs_array
    IFS=' ' read -ra target_archs_array <<< "${TARGET_ARCHS:-x86_64}"
    for target_arch in "${target_archs_array[@]}"; do
        if [[ "${target_arch}" != "${target_archs_array[0]}" ]]; then
            # This is the second (or a later) first pass of a multi-architecture build.
            # We need to prune all leftovers from the previous pass here in order to force re-compilation now.
            make -f "${build_path}/Makefile" -C "${build_path}" distclean
        fi
        qmake "${project_path}" -o "${build_path}/Makefile" \
            "CONFIG+=release" \
            "QMAKE_APPLE_DEVICE_ARCHS=${target_arch}" "QT_ARCH=${target_arch}" \
            "${@:2}"
        make -f "${build_path}/Makefile" -C "${build_path}" -j "${job_count}"
        target_name=$(sed -nE 's/^QMAKE_TARGET *= *(.*)$/\1/p' "${build_path}/Makefile")
        if [[ ${#target_archs_array[@]} -gt 1 ]]; then
            # When building for multiple architectures, move the binary to a safe place to avoid overwriting/cleaning by the other passes.
            mv "${build_path}/${target_name}.app/Contents/MacOS/${target_name}" "${deploy_path}/${target_name}.arch_${target_arch}"
        fi
    done
    if [[ ${#target_archs_array[@]} -gt 1 ]]; then
        echo "Building universal binary from: " "${deploy_path}/${target_name}.arch_"*
        lipo -create -output "${build_path}/${target_name}.app/Contents/MacOS/${target_name}" "${deploy_path}/${target_name}.arch_"*
        rm -f "${deploy_path}/${target_name}.arch_"*

        local file_output
        file_output=$(file "${build_path}/${target_name}.app/Contents/MacOS/${target_name}")
        echo "${file_output}"
        for target_arch in "${target_archs_array[@]}"; do
            if ! grep -q "for architecture ${target_arch}" <<< "${file_output}"; then
                echo "Missing ${target_arch} in file output -- build went wrong?"
                exit 1
            fi
        done
    fi
}

build_app_package() 
{
    local target_name=$(sed -nE 's/^QMAKE_TARGET *= *(.*)$/\1/p' "${build_path}/Makefile")

    # Add Qt deployment dependencies
    if [[ -z "$macadhoc_cert_name" ]]; then
        echo ">>> Doing macdeployqt WITHOUT notarization ..."
        macdeployqt "${build_path}/${target_name}.app" \
            -verbose=2 \
            -always-overwrite \
            -qmldir="${root_path}/src"
    else     # we do this here for signed / notarized dmg ..?
        echo ">>> Doing macdeployqt for notarization ..."
        macdeployqt "${build_path}/${target_name}.app" \
            -verbose=2 \
            -always-overwrite \
            -sign-for-notarization="${macadhoc_cert_name}" \
            -qmldir="${root_path}/src"
    fi

    # debug:
    echo ">>> BUILD FINISHED. Listing of ${build_path}/${target_name}.app/ follows:"
    ls -al ${build_path}/${target_name}.app/

    # copy in provisioning profile
    echo ">>> Adding embedded.provisionprofile to ${build_path}/${target_name}.app/Contents/"
    cp ~/embedded.provisionprofile ${build_path}/${target_name}.app/Contents/
    # echo "${macos_pp}"
    # echo "${macos_pp}" > ${build_path}/${target_name}.app/Contents/embedded.provisionprofile

    # copy app bundle to deploy dir to prep for dmg creation
    # leave original in place for pkg signing if necessary 
    # must use -R to preserve symbolic links
    cp -R "${build_path}/${target_name}.app" "${deploy_path}"

    # # Cleanup
    # make -f "${build_path}/Makefile" -C "${build_path}" distclean

    # Return app name for installer image
    # case "${client_or_server}" in
    #     client_app)
    #         CLIENT_TARGET_NAME="${target_name}"
    #         ;;
    #     # server_app)
    #     #     SERVER_TARGET_NAME="${target_name}"
    #     #     ;;
    #     *)
    #         echo "build_app: invalid parameter '${client_or_server}'"
    #         exit 1
    #         ;;
    # esac
    CLIENT_TARGET_NAME="${target_name}"
}

build_installer_pkg() 
{
    # Get Jamulus version
    local app_version="$(cat "${project_path}" | sed -nE 's/^VERSION *= *(.*)$/\1/p')"
    local target_name=$(sed -nE 's/^QMAKE_TARGET *= *(.*)$/\1/p' "${build_path}/Makefile")

    ## Build installer pkg file - for submission to App Store
    if [[ -z "$macapp_cert_name" ]]; then
        echo ">>> build_installer_pkg: No cert to sign for App Store, bypassing..."
    else
        echo ">>> build_installer_pkg: building with storesign certs...."

        # Clone the build directory to leave the adhoc signed app untouched
        cp -a ${build_path} "${build_path}_storesign"

        # Add Qt deployment deps and codesign the app for App Store submission
        macdeployqt "${build_path}_storesign/${target_name}.app" \
            -verbose=2 \
            -always-overwrite \
            -hardened-runtime -timestamp -appstore-compliant \
            -sign-for-notarization="${macapp_cert_name}" \
            -qmldir="${root_path}/src/"

        echo ">>> Recursive ls in root_dir ...."
        ls -alR

        # Create pkg installer and sign for App Store submission
        productbuild --sign "${macinst_cert_name}" --keychain build.keychain \
            --component "${build_path}_storesign/${target_name}.app" \
            /Applications \
            "${build_path}_storesign/Koord_${app_version}.pkg"  

        # move created pkg file to prep for download
        mv "${build_path}_storesign/Koord_${app_version}.pkg" "${deploypkg_path}"
    fi
}

build_disk_image()
{
    local client_target_name="${1}"
    # local server_target_name="${2}"

    # Install create-dmg via brew. brew needs to be installed first.
    # Download and later install. This is done to make caching possible
    brew_install_pinned "create-dmg" "1.1.0"

    # Get Jamulus version
    local app_version
    app_version=$(sed -nE 's/^VERSION *= *(.*)$/\1/p' "${project_path}")

    # try and test signature of bundle before build
    echo ">>> Testing signature of bundle ...." 
    codesign -vvv --deep --strict "${deploy_path}/Koord.app/"

    # Build installer image
    create-dmg \
      --volname "${client_target_name} Installer" \
      --background "${resources_path}/installerbackground.png" \
      --window-pos 200 400 \
      --window-size 900 320 \
      --app-drop-link 820 210 \
      --text-size 12 \
      --icon-size 72 \
      --icon "${client_target_name}.app" 630 210 \
      --eula "${root_path}/COPYING" \
      "${deploy_path}/${client_target_name}-${app_version}-installer-mac.dmg" \
      "${deploy_path}/"
}

brew_install_pinned() {
    local pkg="$1"
    local version="$2"
    local pkg_version="${pkg}@${version}"
    local brew_bottle_dir="${HOME}/Library/Cache/jamulus-homebrew-bottles"
    local formula="/usr/local/Homebrew/Library/Taps/homebrew/homebrew-cask/Formula/${pkg_version}.rb"
    echo "Installing ${pkg_version}"
    mkdir -p "${brew_bottle_dir}"
    pushd "${brew_bottle_dir}"
    if ! find . | grep -qF "${pkg_version}--"; then
        echo "Building fresh ${pkg_version} package"
        brew developer on  # avoids a warning
        brew extract --version="${version}" "${pkg}" homebrew/cask
        brew install --build-bottle --formula "${formula}"
        brew bottle "${formula}"
        # In order to keep the result the same, we uninstall and re-install without --build-bottle later
        # (--build-bottle is documented to change behavior, e.g. by not running postinst scripts).
        brew uninstall "${pkg_version}"
    fi
    brew install "${pkg_version}--"*
    popd
}

# Check that we are running from the correct location
if [[ ! -f "${project_path}" ]]; then
    echo "Please run this script from the Qt project directory where $(basename "${project_path}") is located."
    echo "Usage: mac/$(basename "${0}")"
    exit 1
fi

# Cleanup previous deployments
cleanup

# Build app
# build_app_compile 
# compile code
build_app_compile_universal
# build .app/ structure
build_app_package 
# create versioned DMG installer image  
build_disk_image "${CLIENT_TARGET_NAME}"

# now build pkg for App store upload
build_installer_pkg


# ##FIXME - only necessary due to SingleApplication / Posix problems 
# # Now build for App Store:
# # - patch app code with SingleApplication patch
# patch -u "${root_path}/src/main.cpp" -i mac/main_posix.patch 
# # rebuild code again
# build_app_compile_universal
# # rebuild .app/ structure
# build_app_package 
# # now build pkg for App store upload
# build_installer_pkg

# make clean
make -f "${build_path}/Makefile" -C "${build_path}" distclean

