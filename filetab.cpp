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
#include "filetab.h"

#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QGridLayout>
#include <QtDebug>



FileTab::FileTab(int iConfiguration, QWidget *parent)
    : QWidget(parent)
    , sBaseDir(QDir::homePath())
    , sOutFileName("data.dat")
    , myConfiguration(iConfiguration)
{
    QString sInfo = QString("%1\n%2\n%3\n%4\n%5").arg(
                            "16451B provides two electrodes:",
                            "  Φ = 38 mm",
                            "  Φ =  5 mm",
                            "Materials should be greater than the",
                            "inner Guard Φ and smaller than Φ 56mm");
    infoEdit.appendPlainText(sInfo);
    infoEdit.setReadOnly(true);
    outFilePathButton.setText(QString("..."));

    // Build the Tab layout
    QGridLayout* pLayout = new QGridLayout();
    pLayout->addWidget(new QLabel("File Path"),            0, 0, 1, 1);
    pLayout->addWidget(&outPathEdit,                       0, 1, 1, 5);
    pLayout->addWidget(&outFilePathButton,                 0, 6, 1, 1);
    pLayout->addWidget(new QLabel("File Name"),            1, 0, 1, 1);
    pLayout->addWidget(&outFileEdit,                       1, 1, 1, 6);
    pLayout->addWidget(new QLabel("Sample Thickness[mm]"), 2, 0, 1, 4);
    pLayout->addWidget(&sampleThicknessEdit,               2, 4, 1, 3);
    pLayout->addWidget(new QLabel("Sample Area[mm^2]"),    3, 0, 1, 4);
    pLayout->addWidget(&sampleAreaEdit,                    3, 4, 1, 3);
    pLayout->addWidget(new QLabel("Sample Information"),   4, 0, 1, 7);
    pLayout->addWidget(&sampleInformationEdit,             5, 0, 4, 7);
    pLayout->addWidget(&infoEdit,                          9, 0, 4, 7);
    setLayout(pLayout);

    sNormalStyle = sampleAreaEdit.styleSheet();

    sErrorStyle  = "QLineEdit { ";
    sErrorStyle += "color: rgb(255, 255, 255);";
    sErrorStyle += "background: rgb(255, 0, 0);";
    sErrorStyle += "selection-background-color: rgb(128, 128, 255);";
    sErrorStyle += "}";

    sInfoStyle   = "QPlainTextEdit { ";
    sInfoStyle  += "color: rgb(0, 0, 0);";
    sInfoStyle  += "background: rgb(255, 255, 0);";
    sInfoStyle  += "selection-background-color: rgb(128, 128, 255);";
    sInfoStyle  += "}";

    connectSignals();
    restoreSettings();
    setToolTips();
    initUI();
}


void
FileTab::initUI() {
    sampleThicknessEdit.setText(sSampleThickness);
    sampleAreaEdit.setText(sSampleArea);
    sampleInformationEdit.setPlainText(sSampleInfo);
    outPathEdit.setText(sBaseDir);
    outFileEdit.setText(sOutFileName);
    infoEdit.setStyleSheet(sInfoStyle);
}


void
FileTab::setToolTips() {
    sampleThicknessEdit.setToolTip(QString("Enter Sample Thickness in mm"));
    sampleAreaEdit.setToolTip(QString("Enter Sample Area in mm^2"));
    sampleInformationEdit.setToolTip(QString("Enter Sample description (multiline)"));
    outPathEdit.setToolTip(QString("Output File Folder"));
    outFileEdit.setToolTip(QString("Enter Output File Name"));
    outFilePathButton.setToolTip((QString("Press to Change Output File Folder")));
}


void
FileTab::connectSignals() {
    connect(&sampleThicknessEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onSampleThicknessTextChanged(const QString)));
    connect(&sampleAreaEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onSampleAreaTextChanged(const QString)));
    connect(&outFilePathButton, SIGNAL(clicked()),
            this, SLOT(on_outFilePathButton_clicked()));
}


void
FileTab::onSampleAreaTextChanged(const QString &sValue) {
    double dTemp = sValue.toDouble();
    bool bValid = dTemp > 0.0;
    if(bValid) {
        sampleAreaEdit.setStyleSheet(sNormalStyle);
        sSampleArea = sValue;
    }
    else {
        sampleAreaEdit.setStyleSheet(sErrorStyle);
    }
}


void
FileTab::onSampleThicknessTextChanged(const QString &sValue) {
    double dTemp = sValue.toDouble();
    bool bValid = dTemp > 0.0;
    if(bValid) {
        sampleThicknessEdit.setStyleSheet(sNormalStyle);
        sSampleThickness = sValue;
    }
    else {
        sampleThicknessEdit.setStyleSheet(sErrorStyle);
    }
}


void
FileTab::restoreSettings() {
    QSettings settings;
    sSampleThickness = settings.value("FileTabSampleThickness", "1.0").toString();
    sSampleArea      = settings.value("FileTabSampleArea", "1.0").toString();
    sSampleInfo      = settings.value("FileTabSampleInfo", "").toString();
    sBaseDir         = settings.value("FileTabBaseDir", sBaseDir).toString();
    sOutFileName     = settings.value("FileTabOutFileName", sOutFileName).toString();
}


void
FileTab::saveSettings() {
    QSettings settings;
    sSampleInfo = sampleInformationEdit.toPlainText();
    settings.setValue("FileTabSampleThickness", sSampleThickness);
    settings.setValue("FileTabSampleArea", sSampleArea);
    settings.setValue("FileTabSampleInfo", sSampleInfo);
    settings.setValue("FileTabBaseDir", sBaseDir);
    settings.setValue("FileTabOutFileName", sOutFileName);
}


void
FileTab::on_outFilePathButton_clicked() {
    QFileDialog chooseDirDialog;
    QDir outDir(sBaseDir);
    chooseDirDialog.setOption(QFileDialog::ShowDirsOnly, true);
    if(outDir.exists())
        chooseDirDialog.setDirectory(outDir);
    else
        chooseDirDialog.setDirectory(QDir::homePath());
    if(chooseDirDialog.exec() == QDialog::Accepted)
        sBaseDir = chooseDirDialog.selectedFiles().at(0);
    outPathEdit.setText(sBaseDir);
}


bool
FileTab::checkFileName() {
    sOutFileName = outFileEdit.text();
    if(sOutFileName == QString()) {
        QMessageBox::information(
                    this,
                    QString("Empty Output Filename"),
                    QString("Please enter a Valid Output File Name"));
        outFileEdit.setFocus();
        return false;
    }
    if(QDir(sBaseDir).exists(sOutFileName)) {
        int iAnswer = QMessageBox::question(
                    this,
                    QString("File Exists"),
                    QString("Do you want overwrite\n%1 ?").arg(sOutFileName),
                    QMessageBox::Yes,
                    QMessageBox::No,
                    QMessageBox::NoButton);
        if(iAnswer == QMessageBox::No) {
            outFileEdit.setFocus();
            return false;
        }
    }
    return true;
}
