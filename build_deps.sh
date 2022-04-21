#!/bin/bash

mkdir deps
cd deps

# Build and install vitaGL, mathneon and vitaShaRK

git clone https://github.com/Rinnegatamante/vitaGL.git
cd vitaGL
make HAVE_SHARK=1 install -j4

cd ../../
rm -rf deps
