#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->joysticks_label->setText("手柄未连接");
    ui->rosbridge_label->setText("rosbridge未连接");
    memset(&m_twist,0,sizeof(m_twist));


    qDebug()<<"【Plugin Joystick】启动";

    QString hostname = "192.168.1.121";

    client_pub = new RosBridgeClient;
    client_pub->SetHostname(hostname);
    client_pub->SetPortWS(9090); //rosbridge 的 服务器 是 9090

    client_pub->ConnectToHost();

    connect(client_pub, &RosBridgeClient::connected, this, [&](){
        ui->rosbridge_label->setText("rosbridge已连接");
    });
    connect(client_pub, &RosBridgeClient::disconnected, this, [&](){
        ui->rosbridge_label->setText("rosbridge未连接");
    });


//    {
//        "op": "publish",
//        "topic": "/turtle1/cmd_vel",
//        "msg": {
//            "linear": {
//                "x": 2.0,
//                "y": 0.0,
//                "z": 0.0
//            },
//            "angular": {
//                "x": 0.0,
//                "y": 0.0,
//                "z": 0.0
//            }
//        }
//    }

    timer = new QTimer;
    QObject::connect(timer, &QTimer::timeout, this,[=]{
        if(client_pub == nullptr){
            return;
        }
        if(client_pub->state() == RosBridgeClient::Connected){

            QJsonObject Twist_data;
            QJsonObject linear;
            linear["x"] = m_twist.linear.x;
            linear["y"] = m_twist.linear.y;
            linear["z"] = m_twist.linear.z;
            QJsonObject angular;
            angular["x"] = m_twist.angular.x;
            angular["y"] = m_twist.angular.y;
            angular["z"] = m_twist.angular.z;

            Twist_data["linear"] = linear;
            Twist_data["angular"] = angular;

            QJsonDocument doc(Twist_data);
            QByteArray cmd = doc.toJson();

            client_pub->publish("/turtle1/cmd_vel",cmd);

        }
    });
    timer->start(50); // 启动定时器 20Hz


    //连接手柄
    m_gamepad = new QGamepad(0);

    connect(m_gamepad,&QGamepad::connectedChanged,this,[=](bool isconnect){
        if(isconnect)
        {
            ui->joysticks_label->setText("手柄已连接");
        }
        else{ui->joysticks_label->setText("手柄未连接");}
    });


//    connect(m_gamepad, &QGamepad::buttonAChanged,worker,&sendfile::sendMsgA);
//    connect(m_gamepad, &QGamepad::buttonBChanged,worker,&sendfile::sendMsgB);
//    connect(m_gamepad, &QGamepad::buttonXChanged,worker,&sendfile::sendMsgX);
//    connect(m_gamepad, &QGamepad::buttonYChanged,worker,sendMsgYbool);
//    connect(m_gamepad, &QGamepad::buttonUpChanged,worker,sendMsgUpbool);
//    connect(m_gamepad, &QGamepad::buttonLeftChanged,worker,&sendfile::sendMsgLeft);
//    connect(m_gamepad, &QGamepad::buttonL1Changed,worker,&sendfile::sendMsgL1);
//    connect(m_gamepad, &QGamepad::buttonR1Changed,worker,&sendfile::sendMsgR1);
//    connect(m_gamepad, &QGamepad::buttonL2Changed,worker,&sendfile::sendMsgL2);
//    connect(m_gamepad, &QGamepad::buttonR2Changed,worker,&sendfile::sendMsgR2);
    connect(m_gamepad, &QGamepad::axisLeftXChanged,this,&MainWindow::sendMsgLeftX);
    connect(m_gamepad, &QGamepad::axisLeftYChanged,this,&MainWindow::sendMsgLeftY);
    connect(m_gamepad, &QGamepad::axisRightXChanged,this,&MainWindow::sendMsgRightX);
    connect(m_gamepad, &QGamepad::axisRightYChanged,this,&MainWindow::sendMsgRightY);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendMsgLeftX(double value)
{
    m_twist.angular.z = value * 0.5;
}

void MainWindow::sendMsgLeftY(double value)
{
    m_twist.linear.x = - value;
}

void MainWindow::sendMsgRightX(double value)
{

}

void MainWindow::sendMsgRightY(double value)
{
    m_twist.linear.z = value;
}

