#!/usr/bin/env bash
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
#   (C) COPYRIGHT 2023 ARM Limited
#       ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.

# ----------------------------------------------------------------------------
# Configuration

# Exit immediately if any component command errors
set -e

BUILD_DIR_64=build_arm64

# ----------------------------------------------------------------------------
# Process command line options
if [ "$#" -lt 1 ]; then
    BUILD_TYPE=Release
else
    BUILD_TYPE=$1
fi

# ----------------------------------------------------------------------------
# Build the 64-bit library
mkdir -p ${BUILD_DIR_64}
pushd ${BUILD_DIR_64}

cmake \
    -DCMAKE_SYSTEM_NAME=Android \
    -DANDROID_PLATFORM=29 \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_TOOLCHAIN=clang \
    -DANDROID_STL=c++_static \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake" \
    -DCMAKE_INSTALL_PREFIX=../ \
    ..

make install -j8

popd
