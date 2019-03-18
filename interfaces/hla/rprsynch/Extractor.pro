
TARGET = Extractor
TEMPLATE = app
CONFIG += thread release

RESOURCES += Extractor.qrc

DEFINES += EXTRACTOR_VERSION=\\\"HLAExtractor-1.0\\\"
DEFINES += USER_DIR=\\\".qualnetUserDir/HLAExtractor-1.0\\\"
DEFINES += _CRT_SECURE_NO_WARNINGS
QMAKE_CXXFLAGS += /wd4100

HEADERS += AdjustSubnets.h	\
           Welcome.h		\
           Setup.h		\
           AdjustNames.h		\
	   Extractor.h	\
	   FileNameWidget.h	\
	   DebugWindow.h	\
           GradientFrame.h

SOURCES += AdjustSubnets.cpp	\
           Welcome.cpp		\
           Setup.cpp		\
           AdjustNames.cpp		\
           Extractor.cpp

FORMS   += AdjustNames.ui	\
           AdjustSubnets.ui	\
           Extract.ui	\
           Setup.ui	\
           Welcome.ui

include(common.pro)
