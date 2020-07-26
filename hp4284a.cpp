#include "hp4284a.h"
#include "utility.h"

#include <gpib/ib.h>
#include <QThread>

namespace hp4284a {
#if !defined(Q_OS_LINUX)
static int rearmMask;
int __stdcall
myCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl, void* callbackData) {
    reinterpret_cast<Keithley236*>(callbackData)->onGpibCallback(LocalUd, LocalIbsta, LocalIberr, LocalIbcntl);
    return rearmMask;
}
#endif
}


Hp4284a::Hp4284a(int gpio, int address, QObject *parent)
    : GpibDevice(gpio, address, parent)
    , hp4284aAddress(address)
    , hp4284a(-1)
{

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
    gpibId = ibdev(gpibNumber, gpibAddress, 0, T3s, 1, 0);
    if(gpibId < 0) {
        QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        emit sendMessage(Q_FUNC_INFO + sError);
        return GPIB_DEVICE_NOT_PRESENT;
    }
    short listen;
    ibln(gpibNumber, gpibAddress, NO_SAD, &listen);
    if(isGpibError(QString(Q_FUNC_INFO) + "HP 4284a Not Respondig"))
        return GPIB_DEVICE_NOT_PRESENT;
    if(listen == 0) {
        ibonl(gpibId, 0);
        emit sendMessage("Nolistener at Addr");
        return GPIB_DEVICE_NOT_PRESENT;
    }
    // set up the asynchronous event notification routine on RQS
#if defined(Q_OS_LINUX)
    connect(&pollTimer, SIGNAL(timeout()),
            this, SLOT(checkNotify()));
    pollTimer.start(pollInterval);
#else
    ibnotify(gpibId,
             RQS,
             (GpibNotifyCallback_t) keithley236::myCallback,
             this);
    if(isGpibError(QString(Q_FUNC_INFO) + "ibnotify call failed."))
        return -1;
#endif
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
        return false;
    }
    sCommand  = "*cls\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "FUNC:IMP:TYPE CPD;:FUNC:IMP:RANG:AUTO ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "DISP:PAGE MEAS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "BIAS:STAT OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "TRIG:SOUR INT\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "INIT:CONT ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "FORM:DATA ASC\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "CORR:OPEN:STATE OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "CORR:SHORT:STATE OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "CORR:LOAD:STATE OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "CORR:LENG 1\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::SetFreq(double Frequency) {
    sCommand =QString("FREQ %1 HZ\r\n").arg(Frequency);
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


void
Hp4284a::GetValues(char* Results, int maxchars) {
    sCommand = QString("FETCH?\r\n");
    gpibWrite(gpibId, sCommand);
    if(ThreadIbsta() & ERR) {
        errmsg(sCommand +" Errore nella Configurazione del'HP4284a", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        GpibError(LastError);
        strcpy(Results, "0.0,0.0,0\n");
        return;
    }
    ibrd(myUd, Results, maxchars);
    if(ThreadIbsta() & ERR) {
        errmsg(" Errore in Lettura dell'HP4284a", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        GpibError(LastError);
        strcpy(Results, "0.0,0.0,0\n");
        return;
    }
    Results[ThreadIbcnt()] = 0;
}


bool
Hp4284a::OpenCorr() {
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "STAT:OPER:ENAB 1\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*SRE 128\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "CORR:OPEN\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::ShortCorr() {
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "STAT:OPER:ENAB 1\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*SRE 128\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "CORR:SHORT\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::CloseCorrection() {
    sCommand = "STAT:OPER:ENAB 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*SRE 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::EnableQuery() {
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "TRIG:SOUR BUS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "INIT:CONT OFF\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "STAT:OPER:ENAB 16\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*SRE 128\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::QueryValues() {
    sCommand = "TRIG\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::DisableQuery() {
    sCommand = "STAT:OPER:ENAB 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*SRE 0\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand = "*CLS\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "TRIG:SOUR INT\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    sCommand  = "INIT:CONT ON\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        return false;
    }
    return true;
}


bool
Hp4284a::SetMode(int Mode) {
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
        return false;
    }
    return true;
}


bool
Hp4284a::GetFreq(double *f) {
    *f = 0.0;
    sCommand = "FREQ?\r\n";
    if(!Configure(Command)) {
        errmsg(Command +" Errore nella Configurazione del'HP4284a", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        GpibError(LastError);
        return false;
    }
    int maxchars = 256;
    char *Results = nullptr;
    Results = new char[maxchars+1];
    ibrd(myUd, Results, maxchars);
    if(ThreadIbsta() & ERR) {
        errmsg(" Errore in Lettura dell'HP4284a", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        GpibError(LastError);
        return false;
    }
    Results[ThreadIbcnt()] = 0;
    *f = atof(Results);
    if(Results != nullptr) delete[]Results;
    return true;
}


void
Hp4284a::SRQService(long ud, long Ibsta, long Iberr, long Ibcntl, long *RearmMask) {
    CHAR spb, Result[1025];
    ibrsp(ud, &spb);
    if (ThreadIbsta() & ERR) {
        if(ThreadIberr() & ESTB) {
            GpibError("Hp4284a Data Lost!");
        } else {
            errmsg("SRQService(Hp4284a): ibrsp() Failed", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
            GpibError(LastError);
        }
        *RearmMask = RQS;
        return;
    }
    sCommand = "STAT:OPER?\r\n";
    gpibWrite(gpibId, sCommand);
    if(isGpibError(QString(Q_FUNC_INFO) + sCommand)) {
        *RearmMask = RQS;
        return;
    }
    ibrd(myUd, Result, 128);
    if(ThreadIbsta() & ERR) {
        errmsg(" Errore in Lettura dell'HP4284a", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        GpibError(LastError);
        *RearmMask = RQS;
        return;
    }
    Result[ThreadIbcnt()] = 0;
    int iReg = atoi(Result);
    if(iReg & CORRECTION_COMPLETE_BIT) {
        pParent->SendMessage(CORRECTION_DONE);
    } else if(iReg & MEASURE_COMPLETE_BIT) {
        pParent->SendMessage(MEASUREMENT_COMPLETE);
    }
    *RearmMask = RQS;
}

