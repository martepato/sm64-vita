#!/bin/bash

mkdir deps
cd deps

# Restore default latest versions of these libraries:

git clone https://github.com/Rinnegatamante/vitaGL.git
cd vitaGL
make install -j$(nproc)
cd ..

git clone https://github.com/Rinnegatamante/vitaShaRK.git
cd vitaShaRK
make install -j$(nproc)
cd ..

cd ..
rm -rf deps
