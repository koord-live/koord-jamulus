#!/bin/sh
cd ..

make distclean
qmake Koord-RT.pro
make
make dist

mkdir -p deploy
mv *.tar.gz deploy/Koord-RT-version.tar.gz
cd linux
