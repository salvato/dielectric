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

#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QTextEdit>


QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(Hp4284a)
QT_FORWARD_DECLARE_CLASS(Plot2D)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(ConfigureDlg)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QStatusBar)


class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(int iBoard, QWidget *parent = nullptr);
    ~MainWindow() override;

public:
    bool checkInstruments();
    void updateUserInterface();


public slots:
    void onConfigure();
    void onStartMeasure();
    void onNew4284Measure();
    void onCorrectionDone();
    void onShowE1();
    void onShowE2();
    void onShowTD();
    void onGpibMessage(QString sMessage);
    void onOpenCorrection();
    void onShortCorrection();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    bool saveLoadCorrectionFile();
    bool saveOpenCorrectionFile();
    bool saveShortCorrectionFile();
    void initPlots();
    void initLayout();
    void saveSettings();
    void getSettings();
    void connectSignals();
    void setToolTips();
    bool prepareLogFile();
    void logMessage(QString sMessage);
    void endMeasure();
    bool prepareOutputFile(QString sBaseDir, QString sFileName);
    void writeHeader();
    void disableButtons(bool bDisable);

private:
    QGridLayout*     pMainLayout;
    QFile*           pOutputFile;
    QFile*           pLogFile;
    Hp4284a*         pHp4284a;
    Plot2D*          pPlotE1_Om;
    Plot2D*          pPlotE2_Om;
    Plot2D*          pPlotTD_Om;
    ConfigureDlg*    pConfigureDlg;
    QCheckBox*       pShowE1_F;
    QCheckBox*       pShowE2_F;
    QCheckBox*       pShowTD_F;
    QStatusBar*      pStatusBar;
    int              gpibBoardID;
    bool	         bPlotE1_Om;
    bool	         bPlotE2_Om;
    bool	         bPlotTD_Om;
    bool             bCanClose;
    QSettings        settings;
    QPushButton      startMeasureButton;
    QPushButton      openCorrectionButton;
    QPushButton      shortCorrectionButton;
    //QPushButton      loadCorrectionButton;
    QString          sNormalStyle;
    QString          sErrorStyle;
    QString          sLogFileName;
    QString          sLogDir;
    int              nFrequencies;
    int              currentFrequencyIndex;
    const double     e0;
    double           c0;
    QVector<double>  frequencies;
    int              stabilizeTime;
};
