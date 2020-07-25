#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QThread>


void
myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtInfoMsg:
    fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    abort();
  }
  fflush(stderr);
}


int
main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QMessageBox msgBox;

    int gpibBoardID = 0;

    QCoreApplication::setOrganizationDomain("Gabriele.Salvato");
    QCoreApplication::setOrganizationName("Gabriele.Salvato");
    QCoreApplication::setApplicationName("Dielectric");
    QCoreApplication::setApplicationVersion("1.0.0");


#ifndef TEST_NO_INTERFACE
    QString sGpibInterface = QString("/dev/gpib%1").arg(gpibBoardID);
    QFileInfo checkFile(sGpibInterface);
    while(!checkFile.exists()) {
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString("No %1 device file").arg(sGpibInterface));
        msgBox.setInformativeText(QString("Is the GPIB Interface connected ? "));
        msgBox.setStandardButtons(QMessageBox::Abort|QMessageBox::Retry);
        msgBox.setDefaultButton(QMessageBox::Retry);
        if(msgBox.exec() == QMessageBox::Abort)
            return 0;
    }
#endif

    MainWindow w(gpibBoardID);
    w.show();
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

#ifndef TEST_NO_INTERFACE
    while(!w.checkInstruments()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString("GPIB Instruments Not Found"));
        msgBox.setInformativeText(QString("Switch on and retry"));
        msgBox.setStandardButtons(QMessageBox::Abort|QMessageBox::Retry);
        msgBox.setDefaultButton(QMessageBox::Retry);
        if(msgBox.exec()==QMessageBox::Abort)
            return 0;
    }
#endif
    //w.updateUserInterface();

    QApplication::restoreOverrideCursor();
    return a.exec();
}
