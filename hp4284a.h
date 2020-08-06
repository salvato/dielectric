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

#pragma once

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
    int     init();
    bool    setMode(int Mode);
    bool    disableQuery();
    bool    queryValues();
    bool    enableQuery();
    bool    closeCorrection();
    void    onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl);
    bool    shortCorrection();
    bool    openCorrection();
    QString getValues();
    bool    setFrequency(double Frequency);
    double  getFrequency();
    bool    setAmplitude(double amplitude);
    double  getAmpltude();
    bool    setAverages(int nAvg);
    int     getAverages();
    bool    setPollInterval(int msPollInterval);
    int     getPollInterval();

signals:
    void correctionDone();
    void measurementComplete();

public slots:
    void checkNotify();

public:
    static const int CPD  =  0; // Sets function to Cp-D
    static const int LPRP =  1; // Sets function to Lp-Rp
    static const int CPQ  =  2; // Sets function to Cp-Q
    static const int LSD  =  3; // Sets function to Ls-D
    static const int CPG  =  4; // Sets function to Cp-G
    static const int LSQ  =  5; // Sets function to Ls-Q
    static const int CPRP =  6; // Sets function to Cp-Rp
    static const int LSRS =  7; // Sets function to Ls-Rs
    static const int CSD  =  8; // Sets function to Cs-D
    static const int RX   =  9; // Sets function to R-X
    static const int CSQ  = 10; // Sets function to Cs-Q
    static const int ZTD  = 11; // Sets function to Z-. (deg)
    static const int CSRS = 12; // Sets function to Cs-Rs
    static const int ZTR  = 13; // Sets function to Z-. (rad)
    static const int LPQ  = 14; // Sets function to Lp-Q
    static const int GB   = 15; // Sets function to G-B
    static const int LPD  = 16; // Sets function to Lp-D
    static const int YTD  = 17; // Sets function to Y-. (deg)
    static const int LPG  = 18; // Sets function to Lp-G
    static const int YTR  = 19; // Sets function to Y-. (rad)

protected:
    bool myInit();

private:

};
