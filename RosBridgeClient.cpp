#include "RosBridgeClient.h"


RosBridgeClient::RosBridgeClient(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");

    m_pWebSocket = new QWebSocket;

    connect(m_pWebSocket,&QWebSocket::binaryMessageReceived,this,&RosBridgeClient::slotOnbinaryMessageReceived);
    connect(m_pWebSocket,&QWebSocket::textMessageReceived,this,&RosBridgeClient::slotOntextMessageReceived);//rosbridge 触发这个
    connect(m_pWebSocket,&QWebSocket::connected,this,&RosBridgeClient::slotOnConnected);
    connect(m_pWebSocket,&QWebSocket::disconnected,this,&RosBridgeClient::slotOnDisConnected);

}

void RosBridgeClient::SetHostname(const QString &hostname)
{
    m_hostname = hostname;
}

void RosBridgeClient::SetPortWS(quint16 port)
{
    m_port = port;
}


void RosBridgeClient::ConnectToHost()
{
    qDebug()<<"RosBridgeClient::ConnectToHost"<<QThread::currentThreadId();
    QString url = "ws://" + m_hostname + ":" + QString::number(m_port);
    m_pWebSocket->open(QUrl(url));
}

void RosBridgeClient::DisconnectFromHost()
{
    m_pWebSocket->close();
}

RosBridgeClient::ClientState RosBridgeClient::state()
{
    QAbstractSocket::SocketState socket_state =  m_pWebSocket->state();

    if(socket_state == QAbstractSocket::ConnectedState ){
        return RosBridgeClient::Connected;
    }
    else if(socket_state == QAbstractSocket::HostLookupState || \
            socket_state == QAbstractSocket::ConnectingState ){
        return RosBridgeClient::Connecting;
    }

    return RosBridgeClient::Disconnected;
}





qint32 RosBridgeClient::publish(const QString &topic, const QByteArray &message)
{
    QJsonObject pub_data;
    pub_data.insert("op","publish");
    pub_data.insert("topic", topic);
    QJsonDocument pub_msg = QJsonDocument::fromJson(message);
    QJsonObject pub_msg_object = pub_msg.object();
    pub_data.insert("msg",pub_msg_object);

    QJsonDocument doc(pub_data);
    QByteArray cmd = doc.toJson();
    return m_pWebSocket->sendBinaryMessage(cmd);
}

qint32 RosBridgeClient::subscribe(const QString &topic)
{
    QJsonObject sub_data;
    sub_data.insert("op","subscribe");
    sub_data.insert("topic", topic);

    QJsonDocument doc(sub_data);
    QByteArray cmd = doc.toJson();
    return m_pWebSocket->sendBinaryMessage(cmd);
}

qint32 RosBridgeClient::callService(const QString &service, const QByteArray &message)
{
    QJsonObject pub_service_data;
    pub_service_data.insert("op","call_service");
    pub_service_data.insert("service", service);
    QJsonDocument pub_service_msg = QJsonDocument::fromJson(message);
    QJsonObject pub_msg_object = pub_service_msg.object();
    pub_service_data.insert("args",pub_msg_object);

    QJsonDocument doc(pub_service_data);
    QByteArray cmd = doc.toJson();
    return m_pWebSocket->sendBinaryMessage(cmd);
}

qint32 RosBridgeClient::setParameters(const QString &node, const QString &parameter, ParameterType type, const QVariant &value)
{
    QString set_parameters_service_name = node + "/set_parameters";

    QJsonObject set_parameters_service_data_value;
    switch (type) {
        case bool_value:
        if(value.type() != QVariant::Bool){
            qWarning() << "Parameter type does not match value!";
        }
        set_parameters_service_data_value.insert("type",1);
        set_parameters_service_data_value.insert("integer_value",value.toBool());
        break;
        case integer_value:
        if(value.type() != QVariant::Int){
            qWarning() << "Parameter type does not match value!";
        }
        set_parameters_service_data_value.insert("type",2);
        set_parameters_service_data_value.insert("integer_value",value.toInt());
        break;
        case double_value:
        if(value.type() != QVariant::Double){
            qWarning() << "Parameter type does not match value!";
        }
        set_parameters_service_data_value.insert("type",3);
        set_parameters_service_data_value.insert("integer_value",value.toDouble());
        break;
        case string_value:
        set_parameters_service_data_value.insert("type",4);
        break;
        case byte_array_value:
        break;
        case bool_array_value:
        break;
        case integer_array_value:
        break;
        case double_array_value:
        break;
        case string_array_value:
        break;
    }
    QJsonObject set_parameters_service_data_parameter_object;
    set_parameters_service_data_parameter_object.insert("name",parameter);
    set_parameters_service_data_parameter_object.insert("value",set_parameters_service_data_value);

    QJsonArray set_parameters_service_data_parameters_array;
    set_parameters_service_data_parameters_array.append(set_parameters_service_data_parameter_object);

    QJsonObject set_parameters_service_data;
    set_parameters_service_data.insert("parameters",set_parameters_service_data_parameters_array);

    QJsonDocument doc(set_parameters_service_data);
    QByteArray set_parameters_service_data_QByteArray = doc.toJson();

    return callService(set_parameters_service_name , set_parameters_service_data_QByteArray);
}

void RosBridgeClient::slotOnbinaryMessageReceived(const QByteArray &message)
{
//    qDebug()<<"slotOnbinaryMessageReceived";

    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject obj = doc.object();
    if(obj["op"].toString() == "subscribe"){ //如果是消息接收
        QString _topic = obj["topic"].toString();
        QJsonObject _sub_msg = obj["msg"].toObject();
        QJsonDocument sub_doc(_sub_msg);
        QByteArray sub_msg = sub_doc.toJson();
        emit messageReceived(sub_msg,_topic);
    }
    else if(obj["op"].toString() == "service_response"){  //如果是服务的回复
        if(obj["result"].toBool() == true){
            QString _service = obj["service"].toString();
            QJsonObject _sub_msg = obj["value"].toObject();
            QJsonDocument sub_doc(_sub_msg);
            QByteArray sub_msg = sub_doc.toJson();
            emit serviceResponse(sub_msg,_service);
        }
    }
}

void RosBridgeClient::slotOntextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject obj = doc.object();
    if(obj["op"].toString() == "publish"){ //如果是消息接收
        QString _topic = obj["topic"].toString();
        QJsonObject _sub_msg = obj["msg"].toObject();
        QJsonDocument sub_doc(_sub_msg);
        QByteArray sub_msg = sub_doc.toJson();
        emit messageReceived(sub_msg,_topic);
    }
    else if(obj["op"].toString() == "service_response"){  //如果是服务的回复
        if(obj["result"].toBool() == true){
            QString _service = obj["service"].toString();
            QJsonObject _sub_msg = obj["value"].toObject();
            QJsonDocument sub_doc(_sub_msg);
            QByteArray sub_msg = sub_doc.toJson();
            emit serviceResponse(sub_msg,_service);
        }
    }
}

void RosBridgeClient::slotOnConnected()
{
    emit connected();
}

void RosBridgeClient::slotOnDisConnected()
{
    emit disconnected();
}


