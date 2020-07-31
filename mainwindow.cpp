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
#include "plot2d.h"
#include "gpibdevice.h"
#include "hp4284a.h"
#include "configuredlg.h"


#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QStatusBar>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QThread>


MainWindow::MainWindow(int iBoard, QWidget *parent)
    : QDialog(parent)
    , pOutputFile(nullptr)
    , pLogFile(nullptr)
    , pHp4284a(nullptr)
    , pPlotE1_Om(nullptr)
    , pPlotE2_Om(nullptr)
    , pPlotTD_Om(nullptr)
    , pConfigureDlg(nullptr)
    , pShowE1_F(nullptr)
    , pShowE2_F(nullptr)
    , pShowTD_F(nullptr)
    , pStatusBar(nullptr)
    , frequencies(nullptr)
    , gpibBoardID(iBoard)
{
    // Init internal variables
    bPlotE1_Om = true;
    bPlotE2_Om = true;
    bPlotTD_Om = true;

    setSizeGripEnabled(false);// To remove the resize-handle in the lower right corner
    setFixedSize(size());// To make the size of the window fixed

    // Prepare message logging
    sLogFileName = QString("dieletricLog.txt");
    sLogDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(!sLogDir.endsWith(QString("/"))) sLogDir+= QString("/");
    sLogFileName = sLogDir+sLogFileName;
    prepareLogFile();

    getSettings();
    initLayout();
    setToolTips();
    pConfigureDlg = new ConfigureDlg(0, this);
    connectSignals();
    initPlots();
    bCanClose = true;
}


MainWindow::~MainWindow() {
}


void
MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    //stopTimers();
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());

    if(pPlotE1_Om)    delete pPlotE1_Om;
    if(pPlotE2_Om)    delete pPlotE2_Om;
    if(pPlotTD_Om)    delete pPlotTD_Om;
    if(pConfigureDlg) delete pConfigureDlg;
    if(pOutputFile)   delete pOutputFile;
    if(pShowE1_F)      delete pShowE1_F;
    if(pShowE2_F)      delete pShowE2_F;
    if(pShowTD_F)      delete pShowTD_F;

    if(pLogFile) {
        if(pLogFile->isOpen()) {
            pLogFile->flush();
        }
        pLogFile->deleteLater();
        pLogFile = nullptr;
    }
}


bool
MainWindow::prepareLogFile() {
    // Rotate 5 previous logs, removing the oldest, to avoid data loss
    QFileInfo checkFile(sLogFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(sLogFileName+QString("_4.txt"));
        for(int i=4; i>0; i--) {
            renamed.rename(sLogFileName+QString("_%1.txt").arg(i-1),
                           sLogFileName+QString("_%1.txt").arg(i));
        }
        renamed.rename(sLogFileName,
                       sLogFileName+QString("_0.txt"));
    }
    // Open the new log file
    pLogFile = new QFile(sLogFileName);
    if (!pLogFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(Q_NULLPTR, "Conductivity",
                                 QString("Unable to open file %1: %2.")
                                 .arg(sLogFileName).arg(pLogFile->errorString()));
        delete pLogFile;
        pLogFile = Q_NULLPTR;
    }
    return true;
}


void
MainWindow::logMessage(QString sMessage) {
    QDateTime dateTime;
    QString sDebugMessage = dateTime.currentDateTime().toString() +
            QString(" - ") +
            sMessage;
    if(pLogFile) {
        if(pLogFile->isOpen()) {
            pLogFile->write(sDebugMessage.toUtf8().data());
            pLogFile->write("\n");
            pLogFile->flush();
        }
        else
            qDebug() << sDebugMessage;
    }
    else
        qDebug() << sDebugMessage;
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
    // Buttons
    startMeasureButton.setText("Start Measure");
    configureButton.setText("Configure");
    // Plots Group
    QGroupBox* pPlotBox = new QGroupBox("Visible Plots");
    pShowE1_F = new QCheckBox(tr("Show E1(F)"));
    pShowE2_F = new QCheckBox(tr("Show E2(F)"));
    pShowTD_F = new QCheckBox(tr("Show TD(F)"));
    pShowE1_F->setChecked(true);
    pShowE2_F->setChecked(true);
    pShowTD_F->setChecked(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(pShowE1_F);
    vbox->addWidget(pShowE2_F);
    vbox->addWidget(pShowTD_F);
    vbox->addStretch(1);
    pPlotBox->setLayout(vbox);
    // Status Bar
    pStatusBar = new QStatusBar();
    pStatusBar->setSizeGripEnabled(false);
    // General Layout
    pLayout->addWidget(&configureButton,    0, 0, 1, 1);
    pLayout->addWidget(&startMeasureButton, 0, 1, 1, 1);
    pLayout->addWidget(pPlotBox,            1, 0, 4, 1);
    pLayout->addWidget(pStatusBar,          5, 0, 1, 2);
    setLayout(pLayout);
}


void
MainWindow::setToolTips() {
}


void
MainWindow::connectSignals() {
    connect(&startMeasureButton, SIGNAL(clicked()),
            this, SLOT(onStartMeasure()));
    connect(&configureButton, SIGNAL(clicked()),
            this, SLOT(onConfigure()));
    connect(pShowE1_F, SIGNAL(clicked()),
            this, SLOT(onShowE1()));
    connect(pShowE2_F, SIGNAL(clicked()),
            this, SLOT(onShowE2()));
    connect(pShowTD_F, SIGNAL(clicked()),
            this, SLOT(onShowTD()));
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
                connect(pHp4284a, SIGNAL(aMessage(QString)),
                        this, SLOT(onGpibMessage(QString)));
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
MainWindow::updateUserInterface() {

}


void
MainWindow::initPlots() {
    pPlotE1_Om = new Plot2D(nullptr, "E'(F)");
    pPlotE2_Om = new Plot2D(nullptr, "E\"(F)");
    pPlotTD_Om = new Plot2D(nullptr, "Tan_Delta(F)");

    pPlotE1_Om->SetLimits(1.0, 10.0, 1.0, 10.0, true, true, true, false);
    pPlotE2_Om->SetLimits(1.0, 10.0, 1.0, 10.0, true, true, true, false);
    pPlotTD_Om->SetLimits(1.0, 10.0, 1.0, 10.0, true, true, true, false);

    pPlotE1_Om->UpdatePlot();
    pPlotE2_Om->UpdatePlot();
    pPlotTD_Om->UpdatePlot();

    if(pShowE1_F->isChecked())
        pPlotE1_Om->show();
    else
        pPlotE1_Om->hide();

    if(pShowE2_F->isChecked())
        pPlotE2_Om->show();
    else
        pPlotE2_Om->hide();

    if(pShowTD_F->isChecked())
        pPlotTD_Om->show();
    else
        pPlotTD_Om->hide();
}



void
MainWindow::onConfigure() {
    pConfigureDlg->exec();
}


void
MainWindow::onStartMeasure() {
    QString sTitle;
    sTitle = startMeasureButton.text();
    if(sTitle == "Stop") {
        endMeasure();
    } else {
        if(pHp4284a->init()) {
            return;
        }
        nFrequencies = initFrequencies();
        pHp4284a->setMode(Hp4284a::CPRP);
        pHp4284a->setFrequency(frequencies[0]);
        pHp4284a->setAmplitude(2.0);

        pPlotE1_Om->ClearPlot();
        pPlotE2_Om->ClearPlot();
        pPlotTD_Om->ClearPlot();

        pPlotE1_Om->NewDataSet(1, 3, QColor(0xFF, 0xFF, 0), Plot2D::iline, "E1(F)");
        pPlotE1_Om->SetShowDataSet(1, true);

        pPlotE2_Om->NewDataSet(1, 3, QColor(0xFF, 0xFF, 0), Plot2D::iline, "E2(F)");
        pPlotE2_Om->SetShowDataSet(1, true);

        pPlotTD_Om->NewDataSet(1, 3, QColor(0xFF, 0xFF, 0), Plot2D::iline, "TanD(F)");
        pPlotTD_Om->SetShowDataSet(1, true);

        startMeasureButton.setText("Stop");
        configureButton.setEnabled(false);

        for(uint i=0; i<nFrequencies; i++) {
            qDebug() << frequencies[i];
            pHp4284a->setFrequency(frequencies[i]);
            pHp4284a->queryValues();
            QString sZvalues = pHp4284a->getValues();
            QStringList sListVal = sZvalues.remove('\n').split(",");
            if(sListVal.count() > 2) {
                if(sListVal[2].toInt() == 0) {
                    pPlotE1_Om->NewPoint(1, frequencies[i], sListVal[0].toDouble());
                    pPlotE2_Om->NewPoint(1, frequencies[i], sListVal[1].toDouble());
                    pPlotTD_Om->NewPoint(1, frequencies[i], sListVal[0].toDouble()/sListVal[1].toDouble());
                    pPlotE1_Om->UpdatePlot();
                    pPlotE2_Om->UpdatePlot();
                    pPlotTD_Om->UpdatePlot();
                }
            }
        }
        qDebug() << "Done";
        endMeasure();
        //sTitle = QString("In Attesa di sZvalues
        //iStatus = STATUS_MEASURING;
    }
}


void
MainWindow::onShowE1() {
    if(pShowE1_F->isChecked())
        pPlotE1_Om->show();
    else
        pPlotE1_Om->hide();
}


void
MainWindow::onShowE2() {
    if(pShowE2_F->isChecked())
        pPlotE2_Om->show();
    else
        pPlotE2_Om->hide();
}


void
MainWindow::onShowTD() {
    if(pShowTD_F->isChecked())
        pPlotTD_Om->show();
    else
        pPlotTD_Om->hide();
}


uint
MainWindow::initFrequencies() {
    if(frequencies) delete frequencies;
    double f0 = 20.0; //atof(pMeasureParCfg->sFMin);
    uint i=0, j=0;
    while(f0 <= 1.0e6) { //atof(pMeasureParCfg->sFMax)) {
        i++;
        f0 *= 2.0;
    }
    if(f0/2.0 < 1.0e6) { //atof(pMeasureParCfg->sFMax)) {
        i++;
    }
    frequencies = new double[i];
    j = i;
    frequencies[0] = 20.0;//atof(pMeasureParCfg->sFMin);
    if(pHp4284a->setFrequency(frequencies[0])) {
        //QThread::msleep(1000);
        frequencies[0] = pHp4284a->getFrequency();
    }
    for(i=1; i<j; i++) {
        frequencies[i] = 2.0 * frequencies[i-1];
        if(frequencies[i] > 1.0e6) frequencies[i] = 1.0e6;
        if(pHp4284a->setFrequency(frequencies[i])) {
            //QThread::msleep(1000);
            frequencies[i] = pHp4284a->getFrequency();
        }
    }
    return j;
}


void
MainWindow::endMeasure() {
    startMeasureButton.setText("Start Measure");
    configureButton.setEnabled(true);
    //pMsg->AddText("Misura Terminata\r\n");
    //iStatus = STATUS_IDLE;
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


void
MainWindow::onGpibMessage(QString sMessage) {
    logMessage(sMessage);
}
