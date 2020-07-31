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
#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "filetab.h"
#include "hp4284tab.h"


QT_FORWARD_DECLARE_CLASS(QGridLayout)


class ConfigureDlg : public QDialog
{
    Q_OBJECT
public:
    ConfigureDlg(int iConfiguration, QWidget *parent);

public:
    hp4284Tab* pTab4284;
    FileTab*   pTabFile;

signals:

public slots:
    void onCancel();
    void onOk();

protected:
    void connectSignals();
    void setToolTips();

private:
    QWidget*          pParent;
    QTabWidget*       pTabWidget;
    QDialogButtonBox* pButtonBox;

    int i4284Index;
    int iFileIndex;
    int configurationType;
};

