#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}


QT_FORWARD_DECLARE_CLASS(LakeShore330)
QT_FORWARD_DECLARE_CLASS(Plot2D)


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
