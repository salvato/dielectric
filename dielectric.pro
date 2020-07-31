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

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += /usr/local/include


# For National Instruments GPIB Boards
LIBS += -L"/usr/local/lib" -lgpib # To include libgpib.so from /usr/local/lib


SOURCES += main.cpp \
    hp4284tab.cpp
SOURCES += gpibdevice.cpp
SOURCES += datastream2d.cpp
SOURCES += plotpropertiesdlg.cpp
SOURCES += configuredlg.cpp
SOURCES += filetab.cpp
SOURCES += hp4284a.cpp
SOURCES += axesdialog.cpp
SOURCES += AxisFrame.cpp
SOURCES += AxisLimits.cpp
SOURCES += DataSetProperties.cpp
SOURCES += plot2d.cpp
SOURCES += mainwindow.cpp

HEADERS += mainwindow.h \
    hp4284tab.h
HEADERS += gpibdevice.h
HEADERS += datastream2d.h
HEADERS += plotpropertiesdlg.h
HEADERS += configuredlg.h
HEADERS += filetab.h
HEADERS += hp4284a.h
HEADERS += axesdialog.h
HEADERS += AxisFrame.h
HEADERS += AxisLimits.h
HEADERS += DataSetProperties.h
HEADERS += plot2d.h

DISTFILES += docs/Agilent_HP4284A.pdf \
    docs/Agilent 16451.pdf
DISTFILES += docs/hp4284a_lcr_manual.pdf
DISTFILES += docs/HP4284A.pdf
DISTFILES += plot.png
