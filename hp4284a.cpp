#include "hp4284a.h"
#include "utility.h"

#if defined(Q_OS_LINUX)
#include <gpib/ib.h>
#else
#include <ni4882.h>
#endif


namespace hp4284a {
int rearmMask;
#if !defined(Q_OS_LINUX)
int __stdcall
myCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl, void* callbackData) {
    reinterpret_cast<Keithley236*>(callbackData)->onGpibCallback(LocalUd, LocalIbsta, LocalIberr, LocalIbcntl);
    return rearmMask;
  }
#endif
}


Hp4284a::Hp4284a(int address, QObject *parent)
    : QObject(parent)
    , hp4284aAddress(address)
    , hp4284a(-1)
{

}
