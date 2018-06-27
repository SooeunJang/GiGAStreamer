#!/bin/bash

RESTSDK_VERSION="v2.9.1"
DEFAULT_LIB_DIRECTORY_PATH="."

libDir=${1:-$DEFAULT_LIB_DIRECTORY_PATH}

install_cpprestsdk(){
	restsdkDir="$libDir/cpprestsdk"
	restsdkBuildDir="$restsdkDir/Release/build.release"
   
   if [ -d "$restsdkDir" ]; then
      rm -rf "$restsdkDir"
   fi
   
	git clone https://github.com/Microsoft/cpprestsdk.git "$restsdkDir"
	(cd $restsdkDir && git checkout tags/$RESTSDK_VERSION -b $RESTSDK_VERSION)
	mkdir "$restsdkBuildDir"
#	if [[ "$OSTYPE" == "linux-gnu" ]]; then
#		export CXX=g++-4.9
#	fi
	(cd "$restsdkBuildDir" && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF)
	(cd "$restsdkBuildDir" && make)
	cp uri.patch ${restsdkDir}/Release/libs/websocketpp/
	cd restsdkBuildDir/libs/websocketpp/
	patch -p0 < uri.patch
}
mkdir -p "$libDir"
install_cpprestsdk
