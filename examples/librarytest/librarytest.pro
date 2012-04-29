
QT       += core network xml
QT       += gui

TEMPLATE = app
CONFIG += console
TARGET = librarytest

SOURCES += ./src/main.cpp

#What does this do?
QMAKE_CXXFLAGS += -D GRIF_CODE_GENERATION=1 -O3


##
# Please fill in GRIFDIR and ROOTDIR with the appropriate paths
# Top directory of GRIF on your machine:
GRIFDIR =C:/grif
# Directory of ROOT on your machine:
ROOTDIR =C:/root
LYNXDIR = "C:\Canberra\LynxCOM_SDKInstaller"

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
INCLUDEPATH += $$ROOTSYSLIB
LIBS += $$ROOTSYSLIB/libCint.lib
LIBS += $$ROOTSYSLIB/libMatrix.lib
LIBS += $$ROOTSYSLIB/libMathCore.lib
LIBS += -L$$ROOTSYSLIB
LIBS += -L$$ROOTSYS/lib -llibCore -llibHist

INCLUDEPATH+= $$LYNXDIR
#LIBS += $$LYNXDIR/1033.dll
LIBS += $$LYNXDIR/Communications.lib
LIBS += $$LYNXDIR/DataTypes.lib
LIBS += $$LYNXDIR/DevCntl.lib
LIBS += $$LYNXDIR/Network.lib
LIBS += $$LYNXDIR/OleHlpr.lib
LIBS += $$LYNXDIR/Serialization.lib
LIBS += $$LYNXDIR/Utility.lib

INCLUDEPATH+="C:/Canberra/SDK/CPPExamples/Examples/Include"
