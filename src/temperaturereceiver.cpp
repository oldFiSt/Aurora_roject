#include "temperaturereceiver.h"

#include <QDebug>

TemperatureReceiver::TemperatureReceiver(QObject *parent)
    : QObject(parent)
{
    m_socket = new QTcpSocket(this);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(2000);
    m_reconnectTimer->setSingleShot(true);

    connect(
        m_socket,
        &QTcpSocket::readyRead,
        this,
        &TemperatureReceiver::readData);

    connect(
        m_socket,
        &QTcpSocket::connected,
        this,
        &TemperatureReceiver::socketConnected);

    connect(
        m_socket,
        &QTcpSocket::disconnected,
        this,
        &TemperatureReceiver::socketDisconnected);

    connect(
        m_socket,
        SIGNAL(error(QAbstractSocket::SocketError)),
        this,
        SLOT(socketError(QAbstractSocket::SocketError)));

    connect(
        m_reconnectTimer,
        &QTimer::timeout,
        this,
        &TemperatureReceiver::tryReconnect);
}

bool TemperatureReceiver::connected() const
{
    return m_connected;
}

void TemperatureReceiver::connectToServer(
    const QString &host,
    int port)
{
    if (host.isEmpty() || port <= 0 || port > 65535)
    {
        emit errorOccurred(
            QStringLiteral("Некорректный адрес или порт"));

        return;
    }

    m_host = host;
    m_port = static_cast<quint16>(port);
    m_autoReconnect = true;

    m_reconnectTimer->stop();

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->abort();

    tryReconnect();
}

void TemperatureReceiver::disconnectFromServer()
{
    m_autoReconnect = false;

    m_reconnectTimer->stop();

    m_socket->abort();

    setConnected(false);
}

void TemperatureReceiver::tryReconnect()
{
    if (!m_autoReconnect)
        return;

    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        return;

    qDebug()
        << "Подключение к Arduino bridge:"
        << m_host
        << m_port;

    m_socket->connectToHost(
        m_host,
        m_port);
}

void TemperatureReceiver::socketConnected()
{
    qDebug()
        << "Соединение с Arduino bridge установлено";

    m_reconnectTimer->stop();

    setConnected(true);
}

void TemperatureReceiver::socketDisconnected()
{
    qDebug()
        << "Соединение с Arduino bridge закрыто";

    setConnected(false);

    if (m_autoReconnect)
        m_reconnectTimer->start();
}

void TemperatureReceiver::socketError(
    QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)

    const QString message =
        m_socket->errorString();

    qDebug()
        << "Ошибка TCP:"
        << message;

    setConnected(false);

    emit errorOccurred(message);

    if (m_autoReconnect &&
        !m_reconnectTimer->isActive())
    {
        m_reconnectTimer->start();
    }
}

void TemperatureReceiver::readData()
{
    m_buffer.append(
        m_socket->readAll());

    while (m_buffer.contains('\n'))
    {
        const int end =
            m_buffer.indexOf('\n');

        const QByteArray line =
            m_buffer.left(end).trimmed();

        m_buffer.remove(
            0,
            end + 1);

        if (line.isEmpty())
            continue;

        bool ok = false;

        const double temperature =
            line.toDouble(&ok);

        if (!ok)
        {
            qDebug()
                << "Некорректные данные от bridge:"
                << line;

            continue;
        }

        if (temperature < -55.0 ||
            temperature > 150.0)
        {
            qDebug()
                << "Температура вне диапазона:"
                << temperature;

            continue;
        }

        qDebug()
            << "Получена температура:"
            << temperature
            << "C";

        emit temperatureReceived(
            temperature);
    }
}

void TemperatureReceiver::setConnected(
    bool connected)
{
    if (m_connected == connected)
        return;

    m_connected = connected;

    emit connectedChanged();
}
