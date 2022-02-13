#!/bin/bash

mkdir deps
cd deps

# Build and install old versions of vitaGL and vitaShaRK

git clone https://github.com/Rinnegatamante/vitaGL.git
cd vitaGL
git reset --hard 7b9de85
make HAVE_SBRK=1 HAVE_SHARK=1 install -j$(nproc)
cd ..

git clone https://github.com/Rinnegatamante/vitaShaRK.git
cd vitaShaRK
git reset --hard 30e2361
make install -j$(nproc)
cd ..

cd ..
rm -rf deps
