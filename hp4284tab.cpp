#include "hp4284tab.h"

#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QGridLayout>
#include <QtDebug>


hp4284Tab::hp4284Tab(QWidget *parent)
    : QWidget(parent)
{

    initUI();

    sNormalStyle = editPollInterval.styleSheet();

    sErrorStyle  = "QLineEdit { ";
    sErrorStyle += "color: rgb(255, 255, 255);";
    sErrorStyle += "background: rgb(255, 0, 0);";
    sErrorStyle += "selection-background-color: rgb(128, 128, 255);";
    sErrorStyle += "}";

    restoreSettings();
    connectSignals();
    setToolTips();
}


void
hp4284Tab::initUI() {

    // Build the Tab layout
    QGridLayout* pLayout = new QGridLayout();

    checkOpenCorrection.setText("Open Correction");
    checkShortCorrection.setText("Short Correction");

    pLayout->addWidget(new QLabel("Poll Interval[ms]"), 0, 0, 1, 1);
    pLayout->addWidget(new QLabel("Test Voltage[V]"),   1, 0, 1, 1);
    pLayout->addWidget(new QLabel("Averages Number"),   2, 0, 1, 1);
    pLayout->addWidget(&checkOpenCorrection,            3, 0, 1, 1);
    pLayout->addWidget(&checkShortCorrection,           4, 0, 1, 1);

    pLayout->addWidget(&editPollInterval,     0, 1, 1, 1);
    pLayout->addWidget(&editVoltage,          1, 1, 1, 1);
    pLayout->addWidget(&editAverages,         2, 1, 1, 1);

    setLayout(pLayout);
}


void
hp4284Tab::setToolTips() {
    editPollInterval.setToolTip(QString("Enter a value [100 - 1000]"));
    editVoltage.setToolTip(QString("Enter a value (0.0 - 2.0]"));
    editAverages.setToolTip(QString("Enter a value [1 - 64]"));
    checkOpenCorrection.setToolTip(QString("Enable/Disable Open Correction"));
    checkShortCorrection.setToolTip(QString("Enable/Disable Short Correction"));
}


void
hp4284Tab::connectSignals() {
    connect(&editPollInterval, SIGNAL(textChanged(const QString)),
            this, SLOT(onPollIntervalTextChanged(const QString)));
    connect(&editVoltage, SIGNAL(textChanged(const QString)),
            this, SLOT(onVoltageTextChanged(const QString)));
    connect(&editAverages, SIGNAL(textChanged(const QString)),
            this, SLOT(onAveragesTextChanged(const QString)));
}


void
hp4284Tab::restoreSettings() {
    QSettings settings;
    editPollInterval.setText(settings.value("hp4284TabPollInterval", "300").toString());
    editVoltage.setText(settings.value("hp4284TabVoltage", "2.0").toString());
    editAverages.setText(settings.value("hp4284TabAverages", "7").toString());
    checkOpenCorrection.setChecked((settings.value("hp4284OpenCorrection", "1")).toInt()!=0);
    checkShortCorrection.setChecked((settings.value("hp4284ShortCorrection", "1")).toInt()!=0);
}


void
hp4284Tab::saveSettings() {
    QSettings settings;
    settings.setValue("hp4284TabPollInterval", editPollInterval.text());
    settings.setValue("hp4284TabVoltage", editVoltage.text());
    settings.setValue("hp4284TabAverages", editAverages.text());
    settings.setValue("hp4284OpenCorrection", checkOpenCorrection.isChecked());
    settings.setValue("hp4284ShortCorrection", checkShortCorrection.isChecked());
}


void
hp4284Tab::setPollInterval(int interval) {
    editPollInterval.setText(QString("%1")
                             .arg(interval));
}


int
hp4284Tab::getPollInterval() {
    return editPollInterval.text().toInt();
}


void
hp4284Tab::setTestVoltage(double voltage) {
    editVoltage.setText(QString("%1").arg(voltage));
}



double
hp4284Tab::getTestVoltage() {
    return editVoltage.text().toDouble();
}


void
hp4284Tab::enableOpenCorrection(bool bEnable) {
    checkOpenCorrection.setChecked(bEnable);
}


bool
hp4284Tab::isOpenCorrectionEnabled() {
    return checkOpenCorrection.isChecked();
}


void
hp4284Tab::enableShortCorrection(bool bEnable) {
    checkShortCorrection.setChecked(bEnable);
}


bool
hp4284Tab::isShortCorrectionEnabled() {
    return checkShortCorrection.isChecked();
}


void
hp4284Tab::onPollIntervalTextChanged(const QString &sValue) {
    int iValue = sValue.toInt();
    if(iValue >= 100 && iValue <= 1000) {
        editPollInterval.setStyleSheet(sNormalStyle);
    }
    else {
        editPollInterval.setStyleSheet(sErrorStyle);
    }
}


void
hp4284Tab::onVoltageTextChanged(const QString &sValue) {
    double dValue = sValue.toDouble();
    if(dValue >= 0.1 && dValue <= 2.0) {
        editVoltage.setStyleSheet(sNormalStyle);
    }
    else {
        editVoltage.setStyleSheet(sErrorStyle);
    }
}


void
hp4284Tab::onAveragesTextChanged(const QString &sValue){
    int iValue = sValue.toInt();
    if(iValue > 0 && iValue < 65) {
        editAverages.setStyleSheet(sNormalStyle);
    }
    else {
        editAverages.setStyleSheet(sErrorStyle);
    }
}



