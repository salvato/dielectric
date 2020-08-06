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

#include "hp4284a.h"

#include <gpib/ib.h>
#include <QThread>
#include <QDebug>

// The HP 4284A offers C-D measurements with a basic accuracy of
// +/- 0.05%(C), +/- 0.0005(D) at all test frequencies with six digit
// resolution


namespace hp4284a {
    static int rearmMask;
    static int CORRECTION_COMPLETE_BIT = 1;
    static int MEASURE_COMPLETE_BIT    = 16;
}


Hp4284a::Hp4284a(int gpio, int address, QObject *parent)
    : GpibDevice(gpio, address, parent)
{
    pollInterval = 500;
}


Hp4284a::~Hp4284a() {
    if(gpibId != -1) {
#if defined(Q_OS_LINUX)
        pollTimer.stop();
        pollTimer.disconnect();
#else
        ibnotify(gpibId, 0, NULL, NULL);// disable notification
#endif
        ibonl(gpibId, 0);// Disable hardware and software.
    }
}


int
Hp4284a::init() {
    gpibId = ibdev(gpibNumber, gpibAddress, 0, T30s, 1, 0);
    if(gpibId < 0) {
        QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        emit aMessage(Q_FUNC_INFO + sError);
        return GPIB_DEVICE_NOT_PRESENT;
    }
    short listen;
    ibln(gpibNumber, gpibAddress, NO_SAD, &listen);
    if(isGpibError(QString(Q_FUNC_INFO) + "HP 4284a Not Respondig"))
        return GPIB_DEVICE_NOT_PRESENT;
    if(listen == 0) {
        ibonl(gpibId, 0);
        emit aMessage("Nolistener at Addr");
        return GPIB_DEVICE_NOT_PRESENT;
    }
    ibclr(gpibId);
    QThread::sleep(1);
    if(!myInit())
        return -1;
    return NO_ERROR;
}


bool
Hp4284a::myInit() {
    sCommand  = "*sre 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "*cls\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "FUNC:IMP:TYPE CPD\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "FUNC:IMP:RANG:AUTO ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "AMPL:ALC ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "DISP:PAGE MEAS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "APER LONG, 7";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "BIAS:STAT OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "TRIG:SOUR INT\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "INIT:CONT ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "FORM:DATA ASC\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "CORR:OPEN:STATE ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "CORR:SHORT:STATE ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "CORR:LOAD:STATE OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "CORR:LENG 1\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    pollTimer.start(pollInterval);
    connect(&pollTimer, SIGNAL(timeout()),
            this, SLOT(checkNotify()));
    return true;
}


QString
Hp4284a::getValues() {
    sCommand = QString("FETCH?\r\n");
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    QString sResults = gpibRead(gpibId);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return sResults;
}


bool
Hp4284a::openCorrection() {
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "STAT:OPER:ENAB 1\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*SRE 128\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "CORR:OPEN\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::shortCorrection() {
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "AMPL:ALC OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "STAT:OPER:ENAB 1\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*SRE 128\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "CORR:SHORT\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::closeCorrection() {
    sCommand = "STAT:OPER:ENAB 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*SRE 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::enableQuery() {
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "TRIG:SOUR BUS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "INIT:CONT OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "STAT:OPER:ENAB 16\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*SRE 128\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::queryValues() {
    sCommand = "TRIG\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::disableQuery() {
    sCommand = "STAT:OPER:ENAB 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*SRE 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "TRIG:SOUR INT\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    sCommand  = "INIT:CONT ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::setMode(int Mode) {
    if((Mode < CPD) || (Mode > YTR)) return false;
    sCommand  = "FUNC:IMP:TYPE ";
    switch (Mode) {
    case CPD:
        sCommand += "CPD";
        break;
    case LPRP:
        sCommand += "LPRP";
        break;
    case CPQ:
        sCommand += "CPQ";
        break;
    case LSD:
        sCommand += "LSD";
        break;
    case CPG:
        sCommand += "CPG";
        break;
    case LSQ:
        sCommand += "LSQ";
        break;
    case CPRP:
        sCommand += "CPRP";
        break;
    case LSRS:
        sCommand += "LSRS";
        break;
    case CSD:
        sCommand += "CPD";
        break;
    case RX:
        sCommand += "RX";
        break;
    case CSQ:
        sCommand += "CSQ";
        break;
    case ZTD:
        sCommand += "ZTD";
        break;
    case CSRS:
        sCommand += "CSRS";
        break;
    case ZTR:
        sCommand += "ZTR";
        break;
    case LPQ:
        sCommand += "LPQ";
        break;
    case GB:
        sCommand += "GB";
        break;
    case LPD:
        sCommand += "LPD";
        break;
    case YTD:
        sCommand += "YTD";
        break;
    case LPG:
        sCommand += "LPG";
        break;
    case YTR:
        sCommand += "YTR";
        break;
    }
    sCommand += "\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


bool
Hp4284a::setFrequency(double Frequency) {
    sCommand = QString("FREQ %1 HZ\r\n").arg(Frequency);
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


double
Hp4284a::getFrequency() {
    sCommand = "FREQ?\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    QString sResults = gpibRead(gpibId);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return sResults.toDouble();
}


bool
Hp4284a::setAmplitude(double amplitude) {
    sCommand =QString("VOLT %1 V\r\n").arg(amplitude);
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return true;
}


double
Hp4284a::getAmpltude() {
    sCommand = "VOLT?\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    QString sResults = gpibRead(gpibId);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    return sResults.toDouble();
}


bool
Hp4284a::setAverages(int nAvg) {
    sCommand = QString("APER LONG, %1").arg(nAvg);
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
        return false;
    }
    return true;
}


int
Hp4284a::getAverages() {
    sCommand = "APER?\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    QStringList sResults = QStringList(gpibRead(gpibId));
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    if(sResults.count() < 2) {
        emit mustExit();
    }
    return sResults.at(1).toInt();
}


bool
Hp4284a::setPollInterval(int msPollInterval) {
    if((msPollInterval >= 100) && (msPollInterval <= 1000)) {
        pollInterval = msPollInterval;
        if(pollTimer.isActive()) {
            pollTimer.setInterval(pollInterval);
            return true;
        }
    }
    return false;
}


int
Hp4284a::getPollInterval() {
    return pollTimer.interval();
}


void
Hp4284a::checkNotify() {
    ibrsp(gpibId, &spollByte);
    if(isGpibError(QString(Q_FUNC_INFO) + "ibrsp() Error"))
        emit mustExit();
    if(!(spollByte & 64))
        return; // SRQ not enabled
    onGpibCallback(gpibId, uint(ThreadIbsta()), uint(ThreadIberr()), ThreadIbcnt());
}


void
Hp4284a::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {
    Q_UNUSED(LocalIbsta)
    Q_UNUSED(LocalIbcntl)
    spollByte = 0;
    int iStatus = ibrsp(LocalUd, &spollByte);
    if(iStatus & ERR) {
        emit aMessage(QString(Q_FUNC_INFO) + QString("GPIB error %1").arg(LocalIberr));
        emit aMessage(QString(Q_FUNC_INFO) + QString("ibrsp() returned: %1").arg(iStatus));
        emit aMessage(QString(Q_FUNC_INFO) + QString("Serial Poll Response Byte %1").arg(spollByte));
    }
    sCommand = "STAT:OPER?\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    QString sResults = gpibRead(gpibId);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        emit mustExit();
    }
    int iReg = sResults.toInt();
    if(iReg & hp4284a::CORRECTION_COMPLETE_BIT) {
        emit correctionDone();
    } else if(iReg & hp4284a::MEASURE_COMPLETE_BIT) {
        emit measurementComplete();
    }
    hp4284a::rearmMask = RQS;
}

