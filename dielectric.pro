#-------------------------------------------------
#
# Project created by QtCreator 2017-12-13T15:05:19
#
#-------------------------------------------------
#MIT License

#Copyright (c) 2017 salvato

#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

QT += core
QT += gui
QT += widgets

TARGET = dielectric
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

windows {
  # For National Instruments GPIB Boards
  INCLUDEPATH += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include"
}

contains(QMAKE_HOST.arch, "armv7l") || contains(QMAKE_HOST.arch, "armv6l"): {
    message("Running on Raspberry")
    INCLUDEPATH += /usr/local/include
}

# For National Instruments GPIB Boards
windows {
  message("Running on Windows")
  LIBS += "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib32/msvc/gpib-32.obj"
}
linux {
  message("Running on Linux")
  LIBS += -L"/usr/local/lib" -lgpib # To include libgpib.so from /usr/local/lib
}

# To include libpigpiod_if2.so from /usr/local/lib
contains(QMAKE_HOST.arch, "armv7l") || contains(QMAKE_HOST.arch, "armv6l"): {
    LIBS += -L"/usr/local/lib" -lpigpiod_if2
}


SOURCES += main.cpp
SOURCES += lakeshore330.cpp
SOURCES += axesdialog.cpp
SOURCES += AxisFrame.cpp
SOURCES += AxisLimits.cpp
SOURCES += cdatastream2d.cpp
SOURCES += DataSetProperties.cpp
SOURCES += plot2d.cpp
SOURCES += utility.cpp
SOURCES += mainwindow.cpp

HEADERS += mainwindow.h
HEADERS += lakeshore330.h
HEADERS += axesdialog.h
HEADERS += AxisFrame.h
HEADERS += AxisLimits.h
HEADERS += cdatastream2d.h
HEADERS += DataSetProperties.h
HEADERS += plot2d.h
HEADERS += utility.h

FORMS += mainwindow.ui
FORMS += axesdialog.ui
