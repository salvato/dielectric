#pragma once

#include <QDialog>
#include <QObject>
#include <QLabel>

class OpenInstructionsDialog : public QDialog
{
    Q_OBJECT

public:
    OpenInstructionsDialog(QWidget *parent);

protected:
    void initLayout();

private:
    QLabel labelHeader;
    QLabel labelImage;
};
