#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}


QT_FORWARD_DECLARE_CLASS(Hp4284a)
QT_FORWARD_DECLARE_CLASS(LakeShore330)
QT_FORWARD_DECLARE_CLASS(Plot2D)


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int iBoard, QWidget *parent = nullptr);
    ~MainWindow() override;

public:
    bool checkInstruments();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Hp4284a*  pHp4284a;
    LakeShore330* pLakeShore;
    Ui::MainWindow *ui;
    int gpibBoardID;
};
