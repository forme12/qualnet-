RTI = $$(RTI_HOME)
isEmpty(RTI) {
    RTI = $$(RTI_HOME_MAK)
    isEmpty(RTI) {
        error( "You must define RTI_HOME or RTI_HOME_MAK")
    }
}

CONFIG += static
QTPLUGIN +=	qjpeg	\
		qgif

TEMPLATE = app

DESTDIR = ../../../bin
!exists( $$DESTDIR ) {
    DESTDIR = ../bin
    }

LICENSE_DIR = ../../../license_dir/windows

QMAKE_CXXFLAGS += -DRTI_USES_STD_FSTREAM
INCLUDEPATH += .
INCLUDEPATH += $${RTI}/include

HEADERS += HLAbase.h	\
           archspec.h	\
           Config.h	\
           configFiles.h	\
           hla_gcclatlon.h	\
           hla_orientation.h	\
           hla_types.h	\
           FedAmb.h	\
           lic.h	\
           RPR_FOM.h	\
           rpr_types.h	\
           sim_types.h

SOURCES += Config.cpp
SOURCES += configFiles.cpp	\
           hla_gcclatlon.cpp	\
           hla_orientation.cpp	\
           hla_types.cpp	\
           FedAmb.cpp	\
           RPR_FOM.cpp \
           rpr_types.cpp	\
           sim_types.cpp

LIBS += -L$${RTI}/lib
win32 {
    LIBS += -llibRTI-NG -llibFedTime
    LMSTRIP_EXE  = ..\..\..\addons\kernel\windows\lmstrip.exe
}
unix {
    LIBS += -lRTI-NG -lfedtime
    LMSTRIP_EXE  = ..\..\..\addons\kernel\linux\lmstrip
}

# include the license code if it's available
LMSTRIP_MAP =  ..\..\..\addons\kernel\windows\lmstrip.map
exists( $$LMSTRIP_EXE ) {
  flexlm {
    release {
        INCLUDEPATH +=  ..\..\..\addons\kernel\machind
        SOURCES += lic.cpp
        QMAKE_CXXFLAGS += -DFLEXLM
        win32 {
            QMAKE_CXXFLAGS_RELEASE = -O2 -MT
            QMAKE_PRE_LINK  = $$LMSTRIP_EXE -mrw $$LMSTRIP_MAP release\lic.obj
            LIBS += /nodefaultlib
            LIBS += -L../../../license_dir/windows
            LIBS += -llmgr
            LIBS += -llibcmt
            LIBS += -llibcpmt
            LIBS += -loldnames
            LIBS += -lkernel32
            LIBS += -luser32
            LIBS += -lnetapi32
            LIBS += -ladvapi32
            LIBS += -lgdi32
            LIBS += -lcomdlg32
            LIBS += -lcomctl32
            LIBS += -lwsock32
        }
        unix {
            QMAKE_PRE_LINK  = $$LMSTRIP_EXE -mrw $$LMSTRIP_MAP lic.o
            LIBS += -L../../../addons/license/linux
            LIBS += -llmgr
        }
    }
  }
}

