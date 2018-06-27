#!/bin/sh
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DADD_WEBSOCKET=TRUE ..
make
echo '----------- build done !-------------'
