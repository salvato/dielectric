#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lakeshore330.h"
#include "plot2d.h"

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // To remove the resize-handle in the lower right corner
    ui->statusBar->setSizeGripEnabled(false);
    // To make the size of the window fixed
    setFixedSize(size());
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
}


MainWindow::~MainWindow() {
    delete ui;
}


void
MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
}


bool
MainWindow::CheckInstruments() {
    SendIFC(gpibBoardID);
    if(isGpibError("MainWindow::CheckInstruments(): SendIFC Error"))
      return false;
    // If addrlist contains only the constant NOADDR, the Universal Device Clear (DCL)
    // message is sent to all the devices on the bus
    Addr4882_t addrlist;
    addrlist = NOADDR;
    DevClearList(gpibBoardID, &addrlist);
    if(isGpibError("MainWindow::CheckInstruments() - DevClearList() failed. Are the Instruments Connected and Switced On ?"))
      return false;

    Addr4882_t padlist[31];
    Addr4882_t resultlist[31];
    for(short i=0; i<30; i++) padlist[i] = i+1;
    padlist[30] = NOADDR;
    FindLstn(gpibBoardID, padlist, resultlist, 30);
    if(isGpibError("MainWindow::CheckInstruments() - FindLstn() failed. Are the Instruments Connected and Switced On ?"))
      return false;
    int nDevices = ThreadIbcnt();
    qInfo() << QString("Found %1 Instruments connected to the GPIB Bus").arg(nDevices);

    // Identify the instruments connected to the GPIB Bus
    QString sCommand, sInstrumentID;
    // Identify the instruments connected to the GPIB Bus
    char readBuf[257];
    for(int i=0; i<nDevices; i++) {
      sCommand = "*IDN?";
      Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
      if(isGpibError("MainWindow::CheckInstruments() - *IDN? Failed"))
        return false;
      Receive(gpibBoardID, resultlist[i], readBuf, 256, STOPend);
      if(isGpibError("MainWindow::CheckInstruments() - Receive() Failed"))
        return false;
      readBuf[ThreadIbcnt()] = '\0';
      sInstrumentID = QString(readBuf);
      qDebug() << QString("Address= %1 - InstrumentID= %2")
                  .arg(resultlist[i])
                  .arg(sInstrumentID);
      if(sInstrumentID.contains("MODEL330", Qt::CaseInsensitive)) {
        if(pLakeShore == NULL)
          pLakeShore = new LakeShore330(gpibBoardID, resultlist[i], this);
      }
      else if(sInstrumentID.contains("4284A", Qt::CaseInsensitive))
      {
        if(pHp4284a == Q_NULLPTR) {
          pHp4284a = new Hp4284a(gpibBoardID, resultlist[i], this);
        }
      }
    }
    if(pHp4284a == Q_NULLPTR) {
      int iAnswer = QMessageBox::warning(this,
                                         "Warning",
                                         "HP4284A LCR Meter Not Connected",
                                         QMessageBox::Abort|QMessageBox::Ignore,
                                         QMessageBox::Abort);
      if(iAnswer == QMessageBox::Abort)
        return false;
    }

    if(pLakeShore == Q_NULLPTR) {
      int iAnswer = QMessageBox::warning(this,
                                         "Warning",
                                         "Lake Shore Not Connected",
                                         QMessageBox::Abort|QMessageBox::Ignore,
                                         QMessageBox::Abort);
      if(iAnswer == QMessageBox::Abort)
        return false;
    }
    return true;
}

