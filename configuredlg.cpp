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

#include "configuredlg.h"

#include <QGridLayout>
#include <QDebug>
#include <QMessageBox>


ConfigureDlg::ConfigureDlg(QWidget *parent)
    : QDialog(parent)
{ 
    getSettings();
    initLayout();
    setToolTips();
    connectSignals();
    bCanClose = true;
}


int
ConfigureDlg::exec() {
    getSettings();
    return QDialog::exec();
}



void
ConfigureDlg::getSettings() {
    restoreGeometry(settings.value(QString("Configuration Dialog")).toByteArray());
}


void
ConfigureDlg::saveSettings() {
    settings.setValue(QString("Configuration Dialog"), saveGeometry());
}


void
ConfigureDlg::initLayout() {
    // Create the Dialog Layout
    QGridLayout* pLayout = new QGridLayout();


    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                      QDialogButtonBox::Cancel);
    pLayout->addWidget(pButtonBox, 7, 0, 1, 4);
    // Set the Layout
    setLayout(pLayout);
}


void
ConfigureDlg::connectSignals() {
    // Button Box
    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));

}


void
ConfigureDlg::onCancel() {
    getSettings();
    reject();
}


void
ConfigureDlg::onOk() {
    if(!bCanClose)
        return;
    saveSettings();
    accept();
}


void
ConfigureDlg::setToolTips() {
}

