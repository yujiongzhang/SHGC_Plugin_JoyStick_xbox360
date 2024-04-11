#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGamepad>
#include <QGamepadManager>
#include "RosBridgeClient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Vector3{
    double x;
    double y;
    double z;
};

struct Twist{
    Vector3 linear;
    Vector3 angular;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QGamepad * m_gamepad;//手柄对象

    RosBridgeClient *client_pub = nullptr;


    void sendMsgLeftX(double value);
    void sendMsgLeftY(double value);
    void sendMsgRightX(double value);
    void sendMsgRightY(double value);
private:
    Ui::MainWindow *ui;
    Twist m_twist;

    QTimer *timer;
};
#endif // MAINWINDOW_H
