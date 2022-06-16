#!/bin/bash
set -eu

root_path=$(pwd)
project_path="${root_path}/Koord-RT.pro"
resources_path="${root_path}/src/res"
build_path="${root_path}/build"
deploy_path="${root_path}/deploy"
deploypkg_path="${root_path}/deploypkg"
cert_name=""
macapp_cert_name=""
macinst_cert_name=""
keychain_pass=""

while getopts 'hs:k:a:i:' flag; do
    case "${flag}" in
        s)
            cert_name=$OPTARG
            if [[ -z "$cert_name" ]]; then
                echo "Please add the name of the certificate to use: -s \"<name>\""
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
        k)
            keychain_pass=$OPTARG
            if [[ -z "$keychain_pass" ]]; then
                echo "Please add keychain password to use: -k \"<name>\""
            fi
            ;;
        h)
            echo "Usage: -s <cert name> for signing mac build"
            exit 0
            ;;
        *)
            exit 1
            ;;
    esac
done

cleanup()
{
    # Clean up previous deployments
    rm -rf "${build_path}"
    rm -rf "${deploy_path}"
    rm -rf "${deploypkg_path}"
    mkdir -p "${build_path}"
    mkdir -p "${deploy_path}"
    mkdir -p "${deploypkg_path}"
}


build_app()
{
    local client_or_server="${1}"

    # Build Jamulus
    qmake "${project_path}" -o "${build_path}/Makefile" "CONFIG+=release" "${@:2}"
    local target_name
    target_name=$(sed -nE 's/^QMAKE_TARGET *= *(.*)$/\1/p' "${build_path}/Makefile")
    local job_count
    job_count=$(sysctl -n hw.ncpu)

    # Get Jamulus version
    local app_version="$(cat "${project_path}" | sed -nE 's/^VERSION *= *(.*)$/\1/p')"

    make -f "${build_path}/Makefile" -C "${build_path}" -j "${job_count}"

    # Add Qt deployment dependencies
    if [[ -z "$cert_name" ]]; then
        macdeployqt "${build_path}/${target_name}.app" \
            -verbose=3 \
            -always-overwrite \
            -hardened-runtime -timestamp -appstore-compliant \
            -qmldir="${root_path}/src"
    else     # we do this here for signed / notarized dmg ..?
        macdeployqt "${build_path}/${target_name}.app" \
            -verbose=3 \
            -always-overwrite \
            -hardened-runtime -timestamp -appstore-compliant \
            -sign-for-notarization="${cert_name}" \
            -qmldir="${root_path}/src"
    fi

    # debug:
    echo ">>> BUILD FINISHED. Listing of ${build_path}/${target_name}.app/ follows:"
    ls -alR ${build_path}/${target_name}.app/

    ## there's no webengine any more! stop this hack!
    # echo ">>> Removing from app: ${build_path}/${target_name}.app/Contents/Frameworks/QtWebEngineCore.framework/"
    # # FIXME - force removal of WebEngine core framework - shouldn't need it and makes pkg 250mb!
    # rm -fr "${build_path}/${target_name}.app/Contents/Frameworks/QtWebEngineCore.framework/"

    # copy app bundle to deploy dir to prep for dmg creation
    # leave original in place for pkg signing if necessary 
    cp -r "${build_path}/${target_name}.app" "${deploy_path}"

    # # Cleanup
    # make -f "${build_path}/Makefile" -C "${build_path}" distclean

    # Return app name for installer image
    case "${client_or_server}" in
        client_app)
            CLIENT_TARGET_NAME="${target_name}"
            ;;
        server_app)
            SERVER_TARGET_NAME="${target_name}"
            ;;
        *)
            echo "build_app: invalid parameter '${client_or_server}'"
            exit 1
    esac
}

# make_clean()
# {
#     # Cleanup
#     make -f "${build_path}/Makefile" -C "${build_path}" distclean
# }

build_installer_pkg() 
{
    # Get Jamulus version
    local app_version="$(cat "${project_path}" | sed -nE 's/^VERSION *= *(.*)$/\1/p')"
    local target_name=$(sed -nE 's/^QMAKE_TARGET *= *(.*)$/\1/p' "${build_path}/Makefile")

    ## Build installer pkg file - for submission to App Store
    if [[ -z "$macapp_cert_name" ]]; then
        echo "No cert to sign for App Store, bypassing..."
    else
        # Clone the build directory to leave the adhoc signed app untouched
        cp -a ${build_path} "${build_path}_storesign"

        # Add Qt deployment deps and codesign the app for App Store submission
        macdeployqt "${build_path}_storesign/${target_name}.app" \
            -verbose=3 \
            -always-overwrite \
            -hardened-runtime -timestamp -appstore-compliant \
            -sign-for-notarization="${macapp_cert_name}" \
            -qmldir="${root_path}/src/"

        echo ">>> Recursive ls in root_dir ...."
        ls -alR
        # echo "Listing the app dir structure"
        # ls -alR "${build_path}_storesign/"

        ## no more webengine, stop this foolishness
        # echo "Removing ${build_path}_storesign/${target_name}.app/Contents/Frameworks/QtWebEngineCore.framework/"
        # # FIXME - force removal of WebEngine core framework - shouldn't need it and makes pkg 250mb!
        # rm -fr "${build_path}_storesign/${target_name}.app/Contents/Frameworks/QtWebEngineCore.framework/"

        # Create pkg installer and sign for App Store submission
        productbuild --sign "${macinst_cert_name}" --keychain build.keychain \
            --component "${build_path}_storesign/${target_name}.app" \
            /Applications \
            "${build_path}_storesign/Koord-RT_${app_version}.pkg"  

        # NOTARIZATION_PASSWORD=""
        # if [ ! -z "$NOTARIZATION_PASSWORD" ]; then
        #     xcrun altool --validate-app -f "${build_path}_storesign/Koord-RT_${app_version}.pkg" -t macos -p @keychain:APPCONNAUTH
        #     xcrun altool --upload-app -f "${build_path}_storesign/Koord-RT_${app_version}.pkg" -t macos -p @keychain:APPCONNAUTH
        # fi

        # move created pkg file to prep for download
        mv "${build_path}_storesign/Koord-RT_${app_version}.pkg" "${deploypkg_path}"
    fi
}

build_disk_image()
{
    local client_target_name="${1}"
    # local server_target_name="${2}"

    # Install create-dmg via brew. brew needs to be installed first.
    # Download and later install. This is done to make caching possible
    brew_install_pinned "create-dmg" "1.0.9"

    # Get Jamulus version
    local app_version
    app_version=$(sed -nE 's/^VERSION *= *(.*)$/\1/p' "${project_path}")

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

# Build Jamulus client and server
# Just build client for Mac
# build_app server_app "CONFIG+=server_bundle"
build_app client_app

build_installer_pkg

# make clean
make -f "${build_path}/Makefile" -C "${build_path}" distclean

# Create versioned installer image 
# build_installer_image "${CLIENT_TARGET_NAME}" "${SERVER_TARGET_NAME}"
build_disk_image "${CLIENT_TARGET_NAME}"
