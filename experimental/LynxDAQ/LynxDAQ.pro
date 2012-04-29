# Copyright (C) 2012 Gamma-ray Imaging Framework Team
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
# 
# The license can be found in the LICENSE.txt file.
# 
# Contact:
# Dr. Daniel Chivers
# dhchivers@lbl.gov

QT       += core network xml
QT       += gui

TARGET = LynxDAQ
CONFIG   += console
TEMPLATE = app

# Source file and headers


## 
# Please fill in GRIFDIR and ROOTDIR with the appropriate paths
# Top directory of GRIF on your machine:
GRIFDIR = C:/Projects/grif
# Directory of ROOT on your machine:
ROOTDIR = C:/root
# Canberra SDK files:


INCLUDEPATH += $$GRIFDIR/experimental/LynxDAQ


QMAKE_CXXFLAGS += -D GRIF_CODE_GENERATION=1 -O3

DEFINES += GRIFPROJECTDIR=$${GRIFPROJECTDIR}
# set up log directory
GRIF_LOG_DIR = $$GRIFPROJECTDIR/log/
win32 {
    GRIF_LOG_DIR = $$GRIFPROJECTDIR\\log
}


DEFINES += GRIF_LOG_DIR=\\\"$${GRIF_LOG_DIR}\\\"
# External libraries
INCLUDEPATH += $$GRIFDIR/external


# ROOT headers
INCLUDEPATH += $$ROOTDIR/include
# ROOT libraries
ROOTSYSLIB += $$ROOTDIR/lib
INCLUDEPATH += $$ROOTSYSLIB
LIBS += -L$$ROOTSYSLIB
LIBS += -L$$ROOTSYS/lib -llibCore -llibHist -llibMatrix -llibMathCore
LIBS += $$ROOTSYSLIB/libCint.lib
LIBS += $$ROOTSYSLIB/libCore.lib
LIBS += $$ROOTSYSLIB/libHist.lib
LIBS += $$ROOTSYSLIB/libMatrix.lib
LIBS += $$ROOTSYSLIB/libMathCore.lib

SOURCES += \
    src/main.cpp

HEADERS +=
