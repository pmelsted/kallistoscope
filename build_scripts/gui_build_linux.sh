#!/bin/bash

#this build script is model after the Bandage build script for linux: https://github.com/rrwick/Bandage 

#the current script  uses dynamic linking, it assumes there is ~/Qt somewhere on the machine
#change to the directory where Qt is stored

#use the qt_dynamic_ubuntu_build to install Qt on the machine
MAKE="$HOME/Qt/5.12.3/gcc_64"

#check if the programs path is already in place
if [ ! -d "kallisto_bustools_gui" ]; then
    #git clone "https://github.com/astamagg/kallisto_bustools_gui.git"
    git clone --single-branch --branch windows https://github.com/astamagg/kallisto_bustools_gui.git
fi

cd kallisto_bustools_gui/

ZIP="kallisto_bustools_gui_linux.zip"

$MAKE/bin/qmake kallisto_bustools_GUI.pro
make

cd ..
zip -j $ZIP kallisto_bustools_gui/kallisto_bustools_GUI
rm -rf kallisto_bustools_gui
