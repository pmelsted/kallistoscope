#!/bin/bash

# IN ORDER TO BE ABLE TO RUN THE SCRIPT THE FOLLOWING NEEDS TO BE IN PLACE:
# * there needs to be a ~/Qt path on the machine to be able to build and run the project. Qt can be downloaded in this path: https://www.qt.io/download
#       The open source version is used in developement
# * to use qt xcode needs to be installed on the machine. Use xcode-select -p to check if xcode is on the machine, if not run xcode-select --install

#creates an app bundle for the kallisto and bustools gui
#based on the Bandage app mac build script, accessable here: https://github.com/rrwick/Bandage/blob/master/build_scripts/bandage_build_mac.sh 
QT_PATH=$(find $HOME/Qt -maxdepth 1 -name "5*" | sort | tail -n 1)

#clone the directory if it doesn't exist
if [ ! -d "kallisto_bustools_gui" ]; then
    #git clone https://github.com/astamagg/kallisto_bustools_gui.git
    git clone --single-branch --branch windows https://github.com/astamagg/kallisto_bustools_gui.git
fi

#create the application
cd kallisto_bustools_gui/
$QT_PATH/clang_64/bin/qmake
make

#add mac dependencies for application to run
$QT_PATH/clang_64/bin/macdeployqt kallisto_bustools_GUI.app -executable=kallisto_bustools_GUI.app/Contents/MacOS/kallisto_bustools_GUI -verbose=3 always-overwrite appstore-compliant

#zip the project in the directory where the build script is located
ZIP="../kallisto_bustools_gui_mac.zip"
ditto -c -k --sequesterRsrc --keepParent kallisto_bustools_GUI.app $ZIP
zip $ZIP

#remove reference to the build
cd ..
rm -rf kallisto_bustools_gui/
