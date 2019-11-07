# kallisto_bustools_gui

## Intro 
This repository provides a user interface for kallisto and bustools. Kallisto is a program for quantifying transcripts from RNA-Seq data. Bustools is a tool to manipulate BUS files
for single cell RNA-Seq datasets. The kallisto_bustools_gui enables the user a platform that performs all the function calls using the data provided by the user.

## Dependencies for building the application
In order to be able to use the application kallisto and bustools need to be set up on the machine in question. The application provides a method to obtain the latest versions of kallisto and bustools. Should said method not work it it neccessary to set the tools up manually using the instructions listed below and restarting the application:
Installation information can be accessed here:</br>
* <a href=https://pachterlab.github.io/kallisto/download>Kallisto</a>
* <a href=https://github.com/BUStools/bustools>Bustools</a>

### Unix application dependencies
For unix machines `curl` and `wget` need to be set up on the machine for the application to work properly. It depends on the operating system how they are installed, but a internet search should provide the necessary instructions.

#### Mac specific installation requirements
To run the application on MacOS xcode needs to be installed through the terminal:</br>
* `xcode-select -p` checks whether xcode is installed on the machine.</br>
  * If not run `xcode-select --install` to install xcode.</br>
  
### Windows application
For the Windows version to work it was necessary to deploy the project folder, the application itself is within the folder when the zip is extracted.

## Building through Qt Creator
If the zip file doesn't work on the machine it is possible in most cases to build the application through Qt Creator. The following steps apply to these procedures:
1. Download the open source option for Qt Creator here: <a href=https://www.qt.io/download>Qt Creator</a>
2. Clone the kallisto_bustools_gui repository: `git clone https://github.com/astamagg/kallisto_bustools_gui.git`
3. Through Qt Creator opt to open a project and search for the kallisto_bustools_GUI.pro file.
4. When the project has been selected press the green play button in the left side lower corner to run the application.
