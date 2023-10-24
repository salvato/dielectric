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
#include "filetab.h"
#include "hp4284tab.h"

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>


ConfigureDlg::ConfigureDlg(int iConfiguration, QWidget *parent)
    : QDialog(parent)
    , pTab4284(nullptr)
    , pTabFile(nullptr)
    , pParent(parent)
    , configurationType(iConfiguration)
{
    pTabWidget   = new QTabWidget();
    pTab4284     = new hp4284Tab(this);
    pTabFile     = new FileTab(configurationType, this);
    i4284Index   = pTabWidget->addTab(pTab4284,  tr("Hp4284a"));
    iFileIndex   = pTabWidget->addTab(pTabFile,  tr("Out File"));

    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                      QDialogButtonBox::Cancel);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pTabWidget);
    mainLayout->addWidget(pButtonBox);
    setLayout(mainLayout);

    connectSignals();
    setToolTips();
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
    if(pTab4284) pTab4284->restoreSettings();
    if(pTabFile) pTabFile->restoreSettings();
    reject();
}


void
ConfigureDlg::onOk() {
    if(!pTabFile->checkFileName()) {
        return;
    }
    pTabFile->saveSettings();
    if(pTab4284) pTab4284->saveSettings();
    pTabWidget->setCurrentIndex(i4284Index);
    accept();
}


void
ConfigureDlg::setToolTips() {
    if(pTabFile)
        pTabWidget->setTabToolTip(iFileIndex, QString("Output File configuration"));
}

