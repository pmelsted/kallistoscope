#!/bin/bash
#downloads and runs the setup up Qt desired version. This is for a dynamic qt version, to build qt statically follow these instructions: https://doc.qt.io/qt-5/linux-deployment.html 
#and a list for the configuration options at: https://doc.qt.io/qt-5/configure-options.html
# Dependencies:
sudo apt-get install zlib1g-dev libxcb1 libxcb1-dev libx11-xcb1 libx11-xcb-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-image0 libxcb-image0-dev libxcb-shm0 libxcb-shm0-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-sync0 libxcb-sync0-dev libxcb-xfixes0-dev libxrender-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-render-util0 libxcb-render-util0-dev libxcb-glx0-dev

# Change this to  the desired version  of the software
VER="5.12.3"

VER2="${VER%.*}"
echo "$VER2"

DOWNLOAD="http://download.qt.io/official_releases/qt/$VER2/$VER/qt-opensource-linux-x64-$VER.run"  
RESULTDIR="qt-opensource-linux-x64-$VER.run"

wget $DOWNLOAD

chmod +x $RESULTDIR

./$RESULTDIR