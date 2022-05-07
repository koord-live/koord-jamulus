#!/bin/bash
set -eu

root_path="$(pwd)"
project_path="${root_path}/Koord-RT.pro"
iosdeploy_path="${root_path}/ios"
resources_path="${root_path}/src/res"
build_path="${root_path}/build"
deploy_path="${root_path}/deploy"
iosdist_cert_name=""
keychain_pass=""

while getopts 'hs:k:' flag; do
    case "${flag}" in
        s)
            iosdist_cert_name=$OPTARG
            if [[ -z "$iosdist_cert_name" ]]; then
                echo "Please add the name of the certificate to use: -s \"<name>\""
            fi
            ;;
        k)
            keychain_pass=$OPTARG
            if [[ -z "$keychain_pass" ]]; then
                echo "Please add keychain password to use: -k \"<name>\""
            fi
            ;;
        h)
            echo "Usage: -s <cert name> for signing ios build"
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
    mkdir -p "${build_path}"
    mkdir -p "${deploy_path}"
}

build_ipa()
{
    ## Builds an ipa file for iOS. Should be run from the repo-root

    # Create Xcode file and build
    qmake -spec macx-xcode Koord-RT.pro

    if [[ -z "$iosdist_cert_name" ]]; then
        /usr/bin/xcodebuild -project Koord-RT.xcodeproj -scheme Koord-RT -configuration Release clean archive \
            -archivePath "build/Koord-RT.xcarchive" \
            CODE_SIGN_IDENTITY="" \
            CODE_SIGNING_REQUIRED=NO \
            CODE_SIGNING_ALLOWED=NO \
            CODE_SIGN_ENTITLEMENTS=""
    else
        /usr/bin/xcodebuild -project Koord-RT.xcodeproj -scheme Koord-RT -configuration Release clean archive \
            -archivePath "build/Koord-RT.xcarchive" \
            -exportOptionsPlist "ios/exportOptionsRelease.plist" \
            CODE_SIGN_IDENTITY="${iosdist_cert_name}" \
            CODE_SIGNING_REQUIRED=YES \
            CODE_SIGNING_ALLOWED=YES \
            CODE_SIGN_ENTITLEMENTS="ios/Koord-RT.entitlements"
    fi

    # Generate ipa by copying the .app file from the xcarchive directory
    mkdir build/Payload
    cp -r build/Koord-RT.xcarchive/Products/Applications/Koord-RT.app build/Payload/
    cd build
    zip -0 -y -r Koord-RT.ipa Payload/

    # copy file
    # mkdir ../deploy
    mv Koord-RT.ipa ../deploy
}

# Cleanup previous deployments
cleanup

# Build ipa file for App Store submission (eg via Transporter etc)
build_ipa