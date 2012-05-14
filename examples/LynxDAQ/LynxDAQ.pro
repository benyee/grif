
QT       += core network xml
QT       += gui

TEMPLATE = app
CONFIG += console
TARGET = LynxDAQ


DEPENDPATH += ../../ . \
        ./src \
        ./include \
        ../../framework/source \
        ../../framework/include \
        ../../framework/source/hist \
        ../../framework/include/hist \

#DEPENDPATH += C:/Canberra/SDK/CPPExamples/Examples/Include


INCLUDEPATH += ../../ . \
        ./include \
        ../../framework/include \
        ../../framework/source \
        ../../framework/source/hist \
        ../../framework/include/hist \

include(../../framework/source/fsource.pri)
include(../../framework/include/finclude.pri)


QMAKE_CXXFLAGS += -D GRIF_CODE_GENERATION=1 -O3


SOURCES += \
    src/main.cpp \
    src/LynxDAQ.cpp \
    src/SIMAnalysisThread.cpp

HEADERS += \
    include/LynxDAQ.h \
    include/SIMAnalysisThread.h

##
# Please fill in GRIFDIR and ROOTDIR with the appropriate paths
# Top directory of GRIF on your machine:
GRIFDIR =C:/grif
# Directory of ROOT on your machine:
ROOTDIR =C:/root
LYNXDIR = "C:/Canberra/LynxCOM_SDKInstaller"

# run code generation
GRIFPROJECTDIR = $$GRIFDIR/examples/LynxDAQ
UTILDIR = $$GRIFDIR/util
system(cd $$UTILDIR)
system(python setup.py $$GRIFPROJECTDIR)
system(cd $$GRIFPROJECTDIR)


GRIF_LOG_DIR = $$GRIFDIR/log/
win32 {
    GRIF_LOG_DIR = $$GRIFDIR\\log
}

DEFINES += GRIF_LOG_DIR=\\\"$${GRIF_LOG_DIR}\\\"
# External libraries
INCLUDEPATH += $$GRIFDIR/external

# ROOT headers
INCLUDEPATH += $$ROOTDIR/include
# ROOT libraries
ROOTSYSLIB += $$ROOTDIR/lib

# All *nix systems
unix|macx {
    LIBS += -L$$ROOTSYSLIB
    LIBS += -L$$ROOTSYS/lib -lCore -lHist -lMatrix -lMathCore
    LIBS += $$ROOTSYSLIB/libCint.so
    LIBS += $$ROOTSYSLIB/libCore.so
    LIBS += $$ROOTSYSLIB/libHist.so
    LIBS += $$ROOTSYSLIB/libMatrix.so
    LIBS += $$ROOTSYSLIB/libMathCore.so
}
# All windows platforms
win32 {
    LIBS += -L$$ROOTSYSLIB
    LIBS += -L$$ROOTSYS/lib -llibCore -llibHist -llibMatrix -llibMathCore -llibCint
}

#Lynx libraries:
INCLUDEPATH+= $$LYNXDIR
LIBS += $$LYNXDIR/1033.lib
LIBS += $$LYNXDIR/Communications.lib
LIBS += $$LYNXDIR/DataTypes.lib
LIBS += $$LYNXDIR/DevCntl.lib
LIBS += $$LYNXDIR/Network.lib
LIBS += $$LYNXDIR/OleHlpr.lib
LIBS += $$LYNXDIR/Serialization.lib
LIBS += $$LYNXDIR/Utility.lib

#Utilities.h:
INCLUDEPATH+="C:/Canberra/SDK/CPPExamples/Examples/Include"
INCLUDEPATH+="C:/Canberra/SDK/CPPExamples/Examples/ExampleList"


#Need the following for some of the functions used in Utilities.h:
LIBS+="C:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib/WS2_32.lib"
LIBS+="C:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib/IPHlpApi.lib"


