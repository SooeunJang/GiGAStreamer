#!/bin/sh
HOME=$(pwd)

if [ ! -d build ] \
then \
	mkdir build \
fi

cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DADD_GSTREAMER=TRUE ..
make
cp ../glsm.config .
if [ "$1" == release ] \
then\
	if [ ! -d release ] \
	then \
	mkdir release \
	cp glsm.config glsm ../release
fi

echo '----------- build done !-------------'
