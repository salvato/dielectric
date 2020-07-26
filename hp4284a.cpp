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
    return NO_ERROR;
}


bool
Hp4284a::SetFreq(double Frequency) {
  sCommand =QString("FREQ %1 HZ\r\n").arg(Frequency);
  if(!Configure(sCommand)) {
    errmsg(Command +" Errore nella Configurazione del'HP4284a", ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    GpibError(LastError);
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

