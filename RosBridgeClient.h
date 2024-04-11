#ifndef ROSBRIDGECLIENT_H
#define ROSBRIDGECLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariant>
#include <QThread>


class RosBridgeClient : public QObject
{
    Q_OBJECT
public:

    enum ClientState {
        Disconnected = 0,
        Connecting,
        Connected
    };

    enum ParameterType
    {
        bool_value = 1,
        integer_value,
        double_value,
        string_value,
        byte_array_value,
        bool_array_value,
        integer_array_value,
        double_array_value,
        string_array_value
    };

    explicit RosBridgeClient(QObject *parent = nullptr);

    void SetHostname(const QString &hostname); //设置rosbridge的ip
    void SetPortWS(quint16 port); // 设置rosbridge的端口，常规为9090

    void ConnectToHost(); // 连接 rosbridge服务器
    void DisconnectFromHost();// 断开rosbridge服务器

    ClientState state(); // 获得当前状态

    qint32 publish(const QString &topic, const QByteArray &message);
    qint32 subscribe(const QString &topic);
    qint32 callService(const QString &service, const QByteArray &message);

    qint32 setParameters(const QString &node, const QString &parameter, ParameterType type, const QVariant &value);


    void slotOnbinaryMessageReceived(const QByteArray &message);
    void slotOntextMessageReceived(const QString &message);
    void slotOnConnected();
    void slotOnDisConnected();



    QWebSocket *m_pWebSocket;

    quint16 m_port;
    QString m_hostname;


signals:
    void messageReceived(QByteArray message ,QString topic);
    void serviceResponse(QByteArray message ,QString service);
    void connected();
    void disconnected();
};

#endif // ROSBRIDGECLIENT_H
