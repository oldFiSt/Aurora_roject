#ifndef TEMPERATURERECEIVER_H
#define TEMPERATURERECEIVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>

class TemperatureReceiver : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected
               READ connected
               NOTIFY connectedChanged)

public:
    explicit TemperatureReceiver(QObject *parent = nullptr);

    bool connected() const;

    Q_INVOKABLE void connectToServer(const QString &host, int port);
    Q_INVOKABLE void disconnectFromServer();

signals:
    void temperatureReceived(double temperature);
    void connectedChanged();
    void errorOccurred(const QString &message);

private slots:
    void readData();
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError error);
    void tryReconnect();

private:
    void setConnected(bool connected);

private:
    QTcpSocket *m_socket = nullptr;
    QTimer *m_reconnectTimer = nullptr;

    QByteArray m_buffer;

    QString m_host;
    quint16 m_port = 0;

    bool m_connected = false;
    bool m_autoReconnect = false;
};

#endif // TEMPERATURERECEIVER_H
