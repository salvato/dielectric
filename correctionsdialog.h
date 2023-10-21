#pragma once

#include <QDialog>
#include <QObject>
#include <QLabel>
#include <QDialogButtonBox>


class CorrectionsDialog : public QDialog
{
    Q_OBJECT

public:
    CorrectionsDialog(QPixmap image, QWidget *parent);

protected:
    void initLayout();

private:
    QPixmap image;
    QLabel labelHeader;
    QLabel labelImage;
    QDialogButtonBox* pButtonBox;
};
