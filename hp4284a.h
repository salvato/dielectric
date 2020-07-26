#ifndef HP4284A_H
#define HP4284A_H

#include <QObject>
#include <QTimer>

#include "gpibdevice.h"


class Hp4284a : public GpibDevice
{
    Q_OBJECT
public:
    explicit Hp4284a(int gpio, int address, QObject *parent = nullptr);
    virtual ~Hp4284a();

public:
    bool GetFreq(double *f);
    bool SetMode(int Mode);
    bool DisableQuery();
    bool QueryValues();
    bool EnableQuery();
    bool CloseCorrection();
    void onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl);
    bool ShortCorr();
    bool OpenCorr();
    void GetValues(char* Results, int maxchars);
    bool SetFreq(double Frequency);

signals:

public slots:
    void checkNotify();

public:
    int      init();

    static const int CPD  =  0;  // Sets function to Cp-D
    static const int LPRP =  1; // Sets function to Lp-Rp
    static const int CPQ  =  2;  // Sets function to Cp-Q
    static const int LSD  =  3;  // Sets function to Ls-D
    static const int CPG  =  4;  // Sets function to Cp-G
    static const int LSQ  =  5;  // Sets function to Ls-Q
    static const int CPRP =  6; // Sets function to Cp-Rp
    static const int LSRS =  7; // Sets function to Ls-Rs
    static const int CSD  =  8;  // Sets function to Cs-D
    static const int RX   =  9;   // Sets function to R-X
    static const int CSQ  = 10;  // Sets function to Cs-Q
    static const int ZTD  = 11;  // Sets function to Z-. (deg)
    static const int CSRS = 12; // Sets function to Cs-Rs
    static const int ZTR  = 13;  // Sets function to Z-. (rad)
    static const int LPQ  = 14;  // Sets function to Lp-Q
    static const int GB   = 15;   // Sets function to G-B
    static const int LPD  = 16;  // Sets function to Lp-D
    static const int YTD  = 17;  // Sets function to Y-. (deg)
    static const int LPG  = 18;  // Sets function to Lp-G
    static const int YTR  = 19;  // Sets function to Y-. (rad)

protected:
    bool myInit();

private:
  int hp4284aAddress;
  int hp4284a;
};

#endif // HP4284A_H
