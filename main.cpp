#include "mainwindow.h"
#include <QApplication>


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

    QCoreApplication::setOrganizationDomain("Gabriele.Salvato");
    QCoreApplication::setOrganizationName("Gabriele.Salvato");
    QCoreApplication::setApplicationName("Dielectric");
    QCoreApplication::setApplicationVersion("1.0.0");

    MainWindow w;
    w.show();

    return a.exec();
}
