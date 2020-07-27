// MIT License

// Copyright (c) 2020 Gabriele Salvato

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plot2d.h"
#include "gpibdevice.h"
#include "hp4284a.h"


#include <QGridLayout>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>


MainWindow::MainWindow(int iBoard, QWidget *parent)
    : QDialog(parent)
    , pPlotE1_Om(nullptr)
    , pPlotE2_Om(nullptr)
    , pPlotTD_Om(nullptr)
    , gpibBoardID(iBoard)
{
    // Init internal variables
    bPlotE1_Om = false;
    bPlotE2_Om = false;
    bPlotTD_Om = false;

    setSizeGripEnabled(false);// To remove the resize-handle in the lower right corner
    setFixedSize(size());// To make the size of the window fixed

    getSettings();
    initLayout();
    setToolTips();
    connectSignals();
    bCanClose = true;
}


MainWindow::~MainWindow() {
    if(pPlotE1_Om) delete pPlotE1_Om;
    if(pPlotE2_Om) delete pPlotE2_Om;
    if(pPlotTD_Om) delete pPlotTD_Om;
}


void
MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
}


void
MainWindow::getSettings() {
    restoreGeometry(settings.value(QString("MainWindow_Dialog")).toByteArray());
}


void
MainWindow::saveSettings() {
    settings.setValue(QString("MainWindow_Dialog"), saveGeometry());
}


void
MainWindow::initLayout() {
    // Create the Dialog Layout
    QGridLayout* pLayout = new QGridLayout();
    configureButton.setText("Configure");
    pLayout->addWidget(&configureButton, 0, 0, 1, 1);
    setLayout(pLayout);
}


void
MainWindow::setToolTips() {
}


void
MainWindow::connectSignals() {
}


bool
MainWindow::checkInstruments() {
    SendIFC(gpibBoardID);
    if(ThreadIbsta() & ERR) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString(Q_FUNC_INFO));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString("SendIFC() Error"));
        msgBox.setInformativeText(QString("Is the GPIB Interface connected ? "));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        Q_UNUSED(ret)
        return false;
    }
    // If addrlist contains only the constant NOADDR,
    // the Universal Device Clear (DCL)
    // message is sent to all the devices on the bus
    Addr4882_t addrlist;
    addrlist = NOADDR;
    DevClearList(gpibBoardID, &addrlist);
    if(ThreadIbsta() & ERR) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString(Q_FUNC_INFO));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString("DevClearList() failed"));
        msgBox.setInformativeText(QString("Are the Instruments Connected and Switched On ?"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        Q_UNUSED(ret)
        return false;
    }
    Addr4882_t padlist[31];
    Addr4882_t resultlist[31];
    for(Addr4882_t i=0; i<30; i++) padlist[i] = i+1;
    padlist[30] = NOADDR;
    FindLstn(gpibBoardID, padlist, resultlist, 30);
    if(ThreadIbsta() & ERR) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString(Q_FUNC_INFO));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString("FindLstn() failed"));
        msgBox.setInformativeText(QString("Are the Instruments Connected and Switched On ?"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        Q_UNUSED(ret)
        return false;
    }
    int nDevices = ThreadIbcnt();
    qInfo() << QString("Found %1 Instruments connected to the GPIB Bus").arg(nDevices);

    // Identify the instruments connected to the GPIB Bus
    QString sCommand, sInstrumentID;
    // Identify the instruments connected to the GPIB Bus
    char readBuf[257];
    for(int i=0; i<nDevices; i++) {
        sCommand = "*IDN?";
        Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
        if(ThreadIbsta() & ERR) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QString(Q_FUNC_INFO));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(QString("*IDN? failed"));
            msgBox.setInformativeText(QString("Are the Instruments Connected and Switched On ?"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();
            Q_UNUSED(ret)
            return false;
        }
        Receive(gpibBoardID, resultlist[i], readBuf, 256, STOPend);
        if(ThreadIbsta() & ERR) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QString(Q_FUNC_INFO));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(QString("Receive() failed"));
            msgBox.setInformativeText(QString("Are the Instruments Connected and Switched On ?"));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();
            Q_UNUSED(ret)
            return false;
        }
        readBuf[ThreadIbcnt()] = '\0';
        sInstrumentID = QString(readBuf);
        qDebug() << QString("Address= %1 - InstrumentID= %2")
                    .arg(resultlist[i])
                    .arg(sInstrumentID);
        if(sInstrumentID.contains("4284A", Qt::CaseInsensitive)) {
            if(pHp4284a == nullptr) {
                pHp4284a = new Hp4284a(gpibBoardID, resultlist[i], this);
            }
        }
    }
    if(pHp4284a == nullptr) {
        int iAnswer = QMessageBox::warning(this,
                                           "Warning",
                                           "HP4284A LCR Meter Not Connected",
                                           QMessageBox::Abort|QMessageBox::Ignore,
                                           QMessageBox::Abort);
        if(iAnswer == QMessageBox::Abort)
            return false;
    }
    return true;
}


void
MainWindow::initPlots() {
    pPlotE1_Om    = new Plot2D(this, "E'(F)");
    pPlotE2_Om    = new Plot2D(this, "E\"(F)");
    pPlotTD_Om  = new Plot2D(this, "Tan_Delta(F)");

    pPlotE1_Om->SetLimits(1.0, 10.0, 1.0, 10.0, true, true, true, false);
    pPlotE2_Om->SetLimits(1.0, 10.0, 1.0, 10.0, true, true, true, false);
    pPlotTD_Om->SetLimits(1.0, 10.0, 1.0, 10.0, true, true, true, false);

    pPlotE1_Om->UpdatePlot();
    pPlotE2_Om->UpdatePlot();
    pPlotTD_Om->UpdatePlot();

    if(bPlotE1_Om)
        pPlotE1_Om->show();
    else
        pPlotE1_Om->hide();

    if(bPlotE2_Om)
        pPlotE2_Om->show();
    else
        pPlotE2_Om->hide();

    if(bPlotTD_Om)
        pPlotTD_Om->show();
    else
        pPlotTD_Om->hide();
}


void
MainWindow::onCorrectionDone() {
/*
    pHp4284a->CloseCorrection();
    bSetup.EnableWindow(true);
    bFileSave.EnableWindow(bDataAvailable);
    bStart.EnableWindow(true);
    bOpenCorr.EnableWindow(true);
    bShortCorr.EnableWindow(true);
    myCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
    SetCursor(myCursor);
*/
}

