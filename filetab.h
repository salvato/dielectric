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


class FileTab : public QWidget
{
    Q_OBJECT
public:
    explicit FileTab(int iConfiguration, QWidget *parent = nullptr);
    void restoreSettings();
    void saveSettings();
    bool checkFileName();

signals:

public slots:
    void on_outFilePathButton_clicked();
    void onSampleAreaTextChanged(const QString &sValue);
    void onSampleThicknessTextChanged(const QString &sValue);

protected:
    void initUI();
    void setToolTips();
    void connectSignals();

public:
    QString sSampleThickness;
    QString sSampleArea;
    QString sSampleInfo;
    QString sBaseDir;
    QString sOutFileName;

private:
    QPlainTextEdit sampleInformationEdit;
    QPlainTextEdit infoEdit;
    QLineEdit      outPathEdit;
    QLineEdit      outFileEdit;
    QPushButton    outFilePathButton;
    QLineEdit      sampleThicknessEdit;
    QLineEdit      sampleAreaEdit;

    // QLineEdit styles
    QString sNormalStyle;
    QString sErrorStyle;
    QString sInfoStyle;

    int            myConfiguration;
};
