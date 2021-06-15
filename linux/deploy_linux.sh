#!/bin/sh
cd ..

make distclean
qmake Koord-Jamulus.pro
make
make dist

mkdir -p deploy
mv *.tar.gz deploy/Jamulus-version.tar.gz
cd linux
