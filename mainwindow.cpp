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
#include <QApplication>

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
    openCorrectionButton.setText("Open Corr.");
    shortCorrectionButton.setText("Short Coor.");
    loadCorrectionButton.setText("Load Corr.");
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
    pLayout->addWidget(&configureButton,       0, 0, 1, 1);
    pLayout->addWidget(&startMeasureButton,    0, 1, 1, 1);
    pLayout->addWidget(&openCorrectionButton,  1, 0, 1, 1);
    pLayout->addWidget(&shortCorrectionButton, 1, 1, 1, 1);
    pLayout->addWidget(&loadCorrectionButton,  1, 2, 1, 1);

    pLayout->addWidget(pPlotBox,               2, 0, 4, 1);
    pLayout->addWidget(pStatusBar,             6, 0, 1, 2);
    setLayout(pLayout);
}


void
MainWindow::disableButtons(bool bDisable) {
    configureButton.setDisabled(bDisable);
    startMeasureButton.setDisabled(bDisable);
    openCorrectionButton.setDisabled(bDisable);
    shortCorrectionButton.setDisabled(bDisable);
    loadCorrectionButton.setDisabled(bDisable);
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
    connect(&openCorrectionButton, SIGNAL(clicked()),
            this, SLOT(onOpenCorrection()));
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
                connect(pHp4284a, SIGNAL(measurementComplete()),
                        this, SLOT(onNew4284Measure()));
                connect(pHp4284a, SIGNAL(correctionDone()),
                        this, SLOT(onCorrectionDone()));
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

    pPlotE1_Om->SetLimits(10.0, 1.0e6, 1.0, 10.0, false, true, true, false);
    pPlotE2_Om->SetLimits(10.0, 1.0e6, 1.0, 10.0, false, true, true, false);
    pPlotTD_Om->SetLimits(10.0, 1.0e6, 1.0, 10.0, false, true, true, false);

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


bool
MainWindow::prepareOutputFile(QString sBaseDir, QString sFileName) {
    if(pOutputFile) {
        pOutputFile->close();
        pOutputFile->deleteLater();
        pOutputFile = Q_NULLPTR;
    }
    pOutputFile = new QFile(sBaseDir + "/" + sFileName);
    if(!pOutputFile->open(QIODevice::Text|QIODevice::WriteOnly)) {
        QMessageBox::critical(this,
                              "Error: Unable to Open Output File",
                              QString("%1/%2")
                              .arg(sBaseDir)
                              .arg(sFileName));
        pStatusBar->showMessage("Unable to Open Output file...");
        return false;
    }
    return true;
}


void
MainWindow::writeHeader() { // Write the File header
    // To cope with the GnuPlot way to handle the comment lines
    // we need a # as a first chraracter in each comment row.
    /*
    pOutputFile->write(QString("%1 %2 %3 %4\n")
                           .arg("#Time[s]", 12)
                           .arg("V[V]", 12)
                           .arg("I[A]", 12)
                           .arg("T[K]", 12)
                           .toLocal8Bit());
        if(pConfigureDlg->pTabK236->bSourceI) {
            pOutputFile->write(QString("# Current=%1[A] Compliance=%2[V]\n")
                               .arg(pConfigureDialog->pTabK236->dStart)
                               .arg(pConfigureDialog->pTabK236->dCompliance).toLocal8Bit());
        }
        else {
            pOutputFile->write(QString("# Voltage=%1[V] Compliance=%2[A]\n")
                               .arg(pConfigureDialog->pTabK236->dStart)
                               .arg(pConfigureDialog->pTabK236->dCompliance).toLocal8Bit());
        }
    }
    else if(bUseHp3478) {
        pOutputFile->write(QString("%1 %2 %3\n")
                           .arg("#Time[s]", 12)
                           .arg("R[Ohm]", 12)
                           .arg("T[K]", 12)
                           .toLocal8Bit());
    }

    QStringList HeaderLines = pConfigureDialog->pTabFile->sSampleInfo.split("\n");
    for(int i=0; i<HeaderLines.count(); i++) {
        pOutputFile->write("# ");
        pOutputFile->write(HeaderLines.at(i).toLocal8Bit());
        pOutputFile->write("\n");
    }
*/
    pOutputFile->flush();
}


void
MainWindow::onStartMeasure() {
    QString sTitle;
    sTitle = startMeasureButton.text();
    if(sTitle == "Stop") {
        endMeasure();
        return;
    }
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    if(pHp4284a->init()) {
        return;
    }
    pHp4284a->setMode(Hp4284a::CPRP); // To be Changed !
    nFrequencies = initFrequencies();
    qDebug() << "N frequencies=" << nFrequencies;
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

    // Open the Output file
    if(!prepareOutputFile(pConfigureDlg->pTabFile->sBaseDir,
                          pConfigureDlg->pTabFile->sOutFileName))
    {
        pStatusBar->showMessage("Unable to Open the Output file...");
        QApplication::restoreOverrideCursor();
        return;
    }
    writeHeader();

    startMeasureButton.setText("Stop");
    configureButton.setEnabled(false);
    currentFrequencyIndex = 0;
    pHp4284a->setFrequency(frequencies[currentFrequencyIndex]);
    pHp4284a->enableQuery();
    pHp4284a->queryValues();
    pStatusBar->showMessage(QString("Waiting data at f=%1Hz").arg(frequencies[currentFrequencyIndex]));
    disableButtons(true);
    startMeasureButton.setEnabled(true);
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
    frequencies = nullptr;
    double f0 = 20.0;
    uint i=0, nF=0;
    while(f0 < 1.0e6) {
        i++;
        f0 *= 2.0;
    }
    qDebug() << i << f0;
    nF = i+1;
    frequencies = new double[nF-1];
    f0 = 20.0;
    if(pHp4284a->setFrequency(f0)) {
        frequencies[0] = pHp4284a->getFrequency();
    }
    for(i=1; i<nF; i++) {
        f0 = 2.0 * f0;
        if(f0 > 1.0e6) f0 = 1.0e6;
        if(pHp4284a->setFrequency(f0)) {
            frequencies[i] = pHp4284a->getFrequency();
        }
    }
    return nF;
}


void
MainWindow::onNew4284Measure() {
    QString sZvalues = pHp4284a->getValues();
    QStringList sListVal = sZvalues.remove('\n').split(",");
    if(sListVal.count() > 2) {
        double f = frequencies[currentFrequencyIndex];
        if(sListVal[2].toInt() == 0) {
            pPlotE1_Om->NewPoint(1, f, sListVal[0].toDouble());
            pPlotE2_Om->NewPoint(1, f, sListVal[1].toDouble());
            pPlotTD_Om->NewPoint(1, f, sListVal[0].toDouble()/sListVal[1].toDouble());
            pPlotE1_Om->UpdatePlot();
            pPlotE2_Om->UpdatePlot();
            pPlotTD_Om->UpdatePlot();
        }
    }
    if(++currentFrequencyIndex >= nFrequencies) {
        endMeasure();
        return;
    }
    pHp4284a->setFrequency(frequencies[currentFrequencyIndex]);
    pHp4284a->queryValues();
    pStatusBar->showMessage(QString("Waiting data at f=%1Hz").arg(frequencies[currentFrequencyIndex]));
}


void
MainWindow::endMeasure() {
    pHp4284a->disableQuery();
    startMeasureButton.setText("Start Measure");
    disableButtons(false);
    QApplication::restoreOverrideCursor();
    pStatusBar->showMessage("Misura Terminata");
    //iStatus = STATUS_IDLE;
}

// The correction function has two kinds of correction methods.
// In one method the open and short correction can be performed
// at all of the frequency points using the interpolation method,
// and in the other method the open, short, and load correction
// can be performed at the frequency points you specify.
// The OPEN correction data is taken at all 48 preset frequency
// points, independent of the test frequency you set. Except for
// those 48 frequency points, the OPEN correction data for each
// measurement point over the speci ed range is calculated using the
// interpolation method
void
MainWindow::onOpenCorrection() {
    QString sTitle;
    sTitle = openCorrectionButton.text();
    if(sTitle == "Stop") {
        onCorrectionDone();
        return;
    }
    if(pHp4284a->init()) {
        return;
    }
    pHp4284a->setMode(Hp4284a::CPRP); // To be Changed !
    pHp4284a->setAmplitude(2.0);
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    if(!pHp4284a->openCorrection())
        return;
    openCorrectionButton.setText("Stop");
    pStatusBar->showMessage("OPEN Correction in progress: Please wait");
}


void
MainWindow::onShortCorr() {
    QString sTitle;
    sTitle = shortCorrectionButton.text();
    if(sTitle == "Stop") {
        onCorrectionDone();
        return;
    }
    if(pHp4284a->init()) {
        return;
    }
    pHp4284a->setMode(Hp4284a::CPRP); // To be Changed !
    pHp4284a->setAmplitude(2.0);
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    if(!pHp4284a->shortCorrection())
        return;
    shortCorrectionButton.setText("Stop");
    pStatusBar->showMessage("SHORT Correction in progress: Please wait");
}

/*
void
MainWindow::OnLoadCorr() {
  CString Title;
  bLoadCorr.GetWindowText(Title);
  if(Title == "Stop") {
    bLoadCorr.SetWindowText("Load Cor.");
    bSetup.EnableWindow(true);
    bFileSave.EnableWindow(bDataAvailable);
    bOpenCorr.EnableWindow(true);
    bShortCorr.EnableWindow(true);
    bStart.EnableWindow(true);
    pMsg->AddText("Load Correction Stopped\r\n");
    iStatus = STATUS_IDLE;
    return;
  }
  char szFilter[] = "Load Correction Data (*.dat)|*.dat|All Files (*.*)|*.*||";
  char szDefExt[] = "dat";
  CFileDialog FileDialog(FALSE, szDefExt, LoadFileName,
                         OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
  if(FileDialog.DoModal() != IDOK) return;
  strcpy(LoadFileName, FileDialog.GetPathName());
  if(!CheckInstruments()) return;
  bSetup.EnableWindow(false);
  bFileSave.EnableWindow(bDataAvailable);
  bLoadCorr.SetWindowText("Stop");
  bOpenCorr.EnableWindow(false);
  bShortCorr.EnableWindow(false);
  bStart.EnableWindow(false);
  nFreq = InitFreq();
  if(Cl != NULL) {delete[] Cl; Cl = NULL;};
  if(Dl != NULL) {delete[] Dl; Dl = NULL;};
  Cl = new double[nFreq];
  Dl = new double[nFreq];
  bCorrectionCompleted = false;
  if(pHp4284a != NULL) pHp4284a->SetMode(CHp4284a::CPD);
  if(pHp4284a != NULL) pHp4284a->SetFreq(GetFirstFreq());
  pMsg->AddText("Measuring Reference Load. Please Wait\r\n" );
  iStatus = STATUS_LOADCORR;
  MeasureCycle();
}
*/

void
MainWindow::onCorrectionDone() {
    pHp4284a->closeCorrection();
    openCorrectionButton.setText("Open Corr.");
    shortCorrectionButton.setText("Short Corr.");
    pStatusBar->showMessage("Correction Done !");
    QApplication::restoreOverrideCursor();
}


void
MainWindow::onGpibMessage(QString sMessage) {
    logMessage(sMessage);
}
