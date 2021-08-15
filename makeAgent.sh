#!/bin/sh

APP_PATH=$PWD

read -p "Build for MUSL?: y/n : " ANSWER
if [ "$ANSWER" = "y" ]; then
    MUSL_BUILD=ON
else
    MUSL_BUILD=OFF
fi

if [ "$MUSL_BUILD" = "ON" ] ; then
    echo "Setting MUSL Toolchain..."
    export CROSS_COMPILE_PATH=/x86_64-linux-musl-cross
    export PATH=$PATH:/x86_64-linux-musl-cross/bin
    export CC=x86_64-linux-musl-gcc
    export CXX=x86_64-linux-musl-g++
fi

read -p "Build SDK?: y/n : " ANSWER
if [ "$ANSWER" = "y" ]; then
    read -p "Install Dependencies for AWS SDK: y/n : " ANSWER
    if [ "$ANSWER" = "y" ]; then
    	apk add make cmake cmake-doc util-linux perl linux-headers
    	apk add git curl unzip vim bash grep
    	apk add zlib-dev
    	apk add curl-dev
    	apk add openssl-dev
        apk update
    fi

    read -p "Download and install MUSL Toolchain : y/n : " ANSWER
    if [ "$ANSWER" = "y" ]; then
        /usr/bin/curl -XGET https://musl.cc/x86_64-linux-musl-cross.tgz -L --output /x86_64-linux-musl-cross.tgz && \
        cd / && tar -xvzf /x86_64-linux-musl-cross.tgz && \
	cd $APP_PATH
    fi

    read -p "Clone AWS SDK Repo : y/n : " ANSWER
    if [ "$ANSWER" = "y" ]; then
        # Clone into AWS Git repo and build it
        git clone --recurse-submodules https://github.com/aws/aws-iot-device-sdk-cpp-v2.git 
        cd aws-iot-device-sdk-cpp-v2/ 
        git checkout v1.13.0 
        git submodule update --init --recursive 
    else
        cd aws-iot-device-sdk-cpp-v2/ 
    fi
    if [ -e aws-sdk-cpp-build ]; then
        read -p "Remove Existing Obj files : y/n : " ANSWER
        if [ "$ANSWER" = "y" ]; then
            rm -rf aws-sdk-cpp-build
        fi
    fi
    mkdir -p aws-sdk-cpp-build 
    cd aws-sdk-cpp-build
    echo "Building SDK... "
    echo "cmake -DCMAKE_INSTALL_PREFIX=${CROSS_COMPILE_PATH}/usr/local -DCMAKE_PREFIX_PATH=${CROSS_COMPILE_PATH}/usr/local -DBUILD_DEPS=ON -DS2N_NO_PQ_ASM=ON -DCMAKE_BUILD_TYPE=\"Release\" .. "
    cmake -DCMAKE_INSTALL_PREFIX=${CROSS_COMPILE_PATH}/usr/local -DCMAKE_PREFIX_PATH=${CROSS_COMPILE_PATH}/usr/local -DBUILD_DEPS=ON -DS2N_NO_PQ_ASM=ON -DCMAKE_BUILD_TYPE="Release" .. 
    pwd; ls -l
    cmake --build . --target install 
    ln -s ${CROSS_COMPILE_PATH}/usr/local/lib64 ${CROSS_COMPILE_PATH}/usr/local/lib
fi


cd $APP_PATH
read -p "Clean build directory?: y/n : " ANSWER
if [ "$ANSWER" = "y" ]; then
    rm -rf build
fi

mkdir -p build
cd build 
read -p "Build Application?"
cmake -DSTATIC_BUILD=ON -DMUSL_BUILD=$MUSL_BUILD ..
cmake --build . 

cd $APP_PATH
