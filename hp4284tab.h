/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#pragma once

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>

class hp4284Tab : public QWidget
{
    Q_OBJECT
public:
    explicit hp4284Tab(QWidget *parent = nullptr);
    void     restoreSettings();
    void     saveSettings();
    void     setPollInterval(int interval);
    int      getPollInterval();
    void     setTestVoltage(double voltage);
    double   getTestVoltage();
    void     enableOpenCorrection(bool bEnable);
    bool     isOpenCorrectionEnabled();
    void     enableShortCorrection(bool bEnable);
    bool     isShortCorrectionEnabled();


public slots:
    void onPollIntervalTextChanged(QString sValue);
    void onVoltageTextChanged(QString sValue);
    void onAveragesTextChanged(QString sValue);


protected:
    void initUI();
    void setToolTips();
    void connectSignals();

private:
    QLineEdit editPollInterval;
    QLineEdit editVoltage;
    QLineEdit editAverages;
    QCheckBox checkOpenCorrection;
    QCheckBox checkShortCorrection;
    // QLineEdit styles
    QString sNormalStyle;
    QString sErrorStyle;
};
