#!/bin/sh
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DADD_GSTREAMER=TRUE ..
make
echo '----------- build done !-------------'
