#include "openinstructionsdialog.h"

#include <QGridLayout>


OpenInstructionsDialog::OpenInstructionsDialog(QWidget *parent)
    : QDialog(parent)
{
    initLayout();
}


void
OpenInstructionsDialog::initLayout() {
    QString sHeader = QString("Configure the test fixture as shown in the image");

    // Create the Dialog Layout
    QGridLayout* pLayout = new QGridLayout();
    labelHeader.setText(sHeader);

    QPixmap image(QString(":/OpenCorrection.png"));
    image = image.scaledToWidth(640);
    labelImage.setPixmap(QPixmap(image));
    pLayout->addWidget(&labelHeader, 0, 0, 1, 1);
    pLayout->addWidget(&labelImage,  2, 0, 1, 1);

    setLayout(pLayout);
}

