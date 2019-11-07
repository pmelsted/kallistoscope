ECHO OFF

rem In order to build the windows version it is necessary to install Micsrosoft Visual Studio: https://visualstudio.microsoft.com/
rem     * With the Visual Studio it is important to install the C++ developer tools
rem Git has the be installed: https://git-scm.com/
rem 7Zip need to be installed: https://www.7-zip.org/
rem And qt needs to be installed:  https://www.qt.io/download

rem The paths need to be updated based on these package placemeeents on the build machine

rem This is a script to deploy the application on Windows 10

rem This svript should be run from the Developer Visual Studio command prompt

rem These variables must be set to the correct paths and values for your system.
set QT_PATH=C:\Qt\5.12.2
set MSVC_PATH='C:\Program Files (x86)\Microsoft Visual Studio\2019\Community'
set MSVC_VERSION=msvc2017
set MSVC_VERSION_NUM=140
set GIT_PATH=C:\Program Files\Git\bin\git
set ZIP_PATH=C:\Program Files\7-Zip\7z.exe

rem Set up the MSVC compiler
call "%MSVC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64


rem source code and use it. If not, it will clone the master branch from GitHub.
if not exist kallisto_bustools_gui\ call "%GIT_PATH%" clone --single-branch --branch windows https://github.com/astamagg/kallisto_bustools_gui.git
call cd kallisto_bustools_gui
call "%QT_PATH%\%MSVC_VERSION%\bin\qmake.exe"
call "%QT_PATH%\..\Tools\QtCreator\bin\jom.exe"
call cd ..

rem Delete all of the source and build files
call move kallisto_bustools_gui\release\kallisto_bustools_gui.exe kallisto_bustools_gui.exe
call rmdir kallisto_bustools_gui\ /S /Q
call mkdir kallisto_bustools_gui\
call move kallisto_bustools_gui.exe kallisto_bustools_gui\kallisto_bustools_gui.exe

rem Add the necessary libraries so the application can be deployed.
call "%QT_PATH%\%MSVC_VERSION%\bin\windeployqt.exe" kallisto_bustools_gui\kallisto_bustools_gui.exe

rem Zip application and remove reference
call "%ZIP_PATH%" a -tzip kallisto_bustools_gui_windows.zip kallisto_bustools_gui\
call rmdir kallisto_bustools_gui\ /S /Q