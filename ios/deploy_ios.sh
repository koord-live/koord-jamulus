#!/bin/bash
set -eu

## Builds an ipa file for iOS. Should be run from the repo-root

# Create Xcode file and build
qmake -spec macx-xcode Koord-RT.pro
/usr/bin/xcodebuild -project Koord-RT.xcodeproj -scheme Koord-RT -configuration Release clean archive -archivePath "build/Koord-RT.xcarchive" CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO CODE_SIGN_ENTITLEMENTS=""

# Generate ipa by copying the .app file from the xcarchive directory
mkdir build/Payload
cp -r build/Koord-RT.xcarchive/Products/Applications/Koord-RT.app build/Payload/
cd build
zip -0 -y -r Koord-RT.ipa Payload/

# Make a deploy folder and copy file
mkdir ../deploy
mv Koord-RT.ipa ../deploy
