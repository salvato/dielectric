#include "hp4284tab.h"

#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QGridLayout>
#include <QtDebug>


hp4284Tab::hp4284Tab(QWidget *parent)
    : QWidget(parent)
{
    // Build the Tab layout
    QGridLayout* pLayout = new QGridLayout();
    pLayout->addWidget(new QLabel("File Path"),          0, 0, 1, 1);
    pLayout->addWidget(new QLabel("File Name"),          1, 0, 1, 1);
    pLayout->addWidget(new QLabel("Sample Information"), 2, 0, 1, 7);
    setLayout(pLayout);

    connectSignals();
    restoreSettings();
    setToolTips();
    initUI();
}


void
hp4284Tab::initUI() {
}


void
hp4284Tab::setToolTips() {
}


void
hp4284Tab::connectSignals() {
}


void
hp4284Tab::restoreSettings() {
    QSettings settings;
}


void
hp4284Tab::saveSettings() {
    QSettings settings;
}
