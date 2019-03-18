Building the Standard Interfaces Library - HLA Interface

1. General Instructions
The Standard Interfaces Library HLA interface supports the MaK RTI on Windows and Linux. However, any RTI implementation that follows the HLA 1.3 standard should work but may require some custom settings.

Before trying to build the HLA Interface you must install a RTI that provides the RTI C++ header files and libraries for development. You cannot build the Standard Interfaces Library with a java only RTI or if you have only installed the RTI runtime libraries.

To enable a HLA Interface build, you need to edit the addons makefile for your platform. This is located in the main subdirectory and is either Makefile-addons-unix or Makefile-addons-windows. In the addons makefile, the "include ../interface/hla/Makefile-windows" line should be uncommented.

To allow the compiler to find the RTI header files and libraries, you must set the RTI_HOME environment variable with the path to the RTI installation directory. For most RTI implementations, this is the only setting necessary before building the Standard Interfaces Library.

Recompile QualNet by first running "make clean" and then "make" in the main subdirectory using the appropriate make program and makefile. For example, on windows with Visual C++ 8.0 you would use
    nmake -f Makefile-windows-vc8 clean
    nmake -f Makefile-windows-vc8

2. Detailed Instructions
Some RTI implementations install their include files and libraries in special subdirectories or may even install multiple platform or compiler specific versions. The HLA interface build uses 6 variables to set compiler and linker options. In most cases, these have default values that do not need to be changed.

RTI_HOME - The RTI install directory. This must be set before building the HLA Interface.

RTI_INCLUDE_DIR - The directory containing the RTI C++ header files. The default is
    RTI_INCLUDE_DIR=$(RTI_HOME)/include.

RTI_DEFINES - HLA specific compiler options. The default is
    RTI_DEFINES=-DRTI_USES_STD_FSTREAM.

RTI_LIB_DIR - The directory containing the RTI development libraries. This may not be the same as the directory containing the RTI runtime libraries. The default is 
    RTI_LIB_DIR=$(RTI_HOME)/lib

RTI_LIBRARY - Options for linking with the RTI library. The default depends on the type of build.

Windows 32 bit
    RTI_LIBRARY=libRTI-NG.lib libFedTime.lib

Windows 64 bit
    RTI_LIBRARY=libRTI-NG_64.lib libFedTime_64.lib

Unix
    RTI_LIBRARY=-lRTI_NG -lfedtime

RTI_ADDITIONAL_LIBRARIES - Additional link options. The default is no additional link options.

Consult your RTI documentation for details on the options and libraries required for compiling and linking.

3. Mak RTI

At the time this document was written, the Mak RTI supports Windows 32 bit, Windows 64 bit and Linux 32 bit. It does not support 64bit linux or MacOS. Building the HLA Interface using the MaK RTI does not require any customization.

To use the Mak RTI debug libraries on windows you would set the following before building the HLA Interface
    set "RTI_LIBRARY=libRTI-NGd.lib libfedtimed.lib"

Note: Some 64 bit installations of the Mak RTI are missing the iconv_64.dll. You should contact Mak support to get this file.

4. PoRTIco
http://porticoproject.org

PoRTIco is an open source, java based RTI with a C++ wrapper. It requires Java to be installed. The PoRTIco libraries need to be able to find the Java Virtual Machine library (libjvm.). On most platforms, you can build the HLA Interface without explicitly telling the linker to include libjvm. If you have a special development environment, you may need to explicitly tell the linker where to find libjvm.

The default PoRTIco installation is 32 bit only. If you wish to use 64 bit you will need to get the PoRTIco sources and rebuild it.

PoRTIco places the RTI header files in $(RTI_HOME)\include\ng6.

Windows
IF you have PoRTIco version 1.0.1 installed on windows in C:\Program Files\Portico, you would set the following

    set "RTI_HOME=C:\Program Files\Portico\portico-1.0.1"
    set "RTI_INCLUDE_DIR=C:\Program Files\Portico\portico-1.0.1\include\ng6"

Linux
The following are the settings you would use for when both Portico and JDK are installed in /usr/local.

export LD_LIBRARY_PATH=/usr/local/jdk1.6.0_16/jre/lib/i386/client/:/usr/local/portico-1.0.1/lib
    export RTI_HOME=/usr/local/portico-1.0.1
    export RTI_INCLUDE_DIR=/usr/local/portico-1.0.1/include/ng6
    export "RTI_LIBRARY=-lRTI-NG -lFedTime"

MacOS
The following are the settings you would use for when Portico is installed in /usr/local and you are using the default MacOS java runtime.
    export RTI_HOME=/usr/local/portico-1.0.1
    export RTI_INCLUDE_DIR=/usr/local/portico-1.0.1/include/ng6/
    export RTI_LIBRARY=-lRTI-NG

Note: At run time set
    export DYLD_LIBRARY_PATH=/usr/local/portico-1.0.1/lib

3. Troubleshooting
If you experience errors compiling or linking the HLA Interface, the following may be helpful.

When setting environment variables that contain spaces, place quotes outside the entire expression. For example
    set "RTI_HOME=C:\Program Files\Portico\portico-0.8"
    export "RTI_LIBRARY=-lRTI-NG -lFedTime"

On some Unix systems, depending on the version of the linker used, you must add the path to the RTI library directory to the LD_LIBRARY_PATH environment variable for the linker to find all of the dependencies for the RTI libraries.

