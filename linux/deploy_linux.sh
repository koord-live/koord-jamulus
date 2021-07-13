#!/bin/sh
cd ..

make distclean
qmake Koord-RealTime.pro
make
make dist

mkdir -p deploy
mv *.tar.gz deploy/Koord-RealTime-version.tar.gz
cd linux
