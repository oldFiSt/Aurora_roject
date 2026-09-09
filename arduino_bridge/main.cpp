#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QPointer>
#include <QTimer>
#include <QDebug>

#include <windows.h>


static HANDLE openArduinoPort(
    const QString &portName)
{
    QString fullPortName =
        "\\\\.\\" + portName;

    HANDLE handle = CreateFileW(
        reinterpret_cast<LPCWSTR>(
            fullPortName.utf16()
            ),
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
        );

    if (handle == INVALID_HANDLE_VALUE)
    {
        return INVALID_HANDLE_VALUE;
    }


    DCB dcb = {};

    dcb.DCBlength =
        sizeof(DCB);


    if (!GetCommState(
            handle,
            &dcb))
    {
        CloseHandle(handle);

        return INVALID_HANDLE_VALUE;
    }


    dcb.BaudRate =
        CBR_9600;

    dcb.ByteSize =
        8;

    dcb.Parity =
        NOPARITY;

    dcb.StopBits =
        ONESTOPBIT;


    if (!SetCommState(
            handle,
            &dcb))
    {
        CloseHandle(handle);

        return INVALID_HANDLE_VALUE;
    }


    COMMTIMEOUTS timeouts = {};

    timeouts.ReadIntervalTimeout =
        1;

    timeouts.ReadTotalTimeoutConstant =
        1;

    timeouts.ReadTotalTimeoutMultiplier =
        0;


    SetCommTimeouts(
        handle,
        &timeouts
        );


    PurgeComm(
        handle,
        PURGE_RXCLEAR
            | PURGE_TXCLEAR
        );


    return handle;
}



int main(
    int argc,
    char *argv[])
{
    QCoreApplication app(
        argc,
        argv
        );


    // =====================================================
    // НАСТРОЙКИ
    // =====================================================

    QString comPort =
        "COM13";

    quint16 tcpPort =
        5555;


    // Можно запустить:
    //
    // arduino_bridge.exe COM13
    //
    // или:
    //
    // arduino_bridge.exe COM13 5555

    if (app.arguments().size() >= 2)
    {
        comPort =
            app.arguments().at(1);
    }


    if (app.arguments().size() >= 3)
    {
        bool ok = false;

        int value =
            app.arguments()
                .at(2)
                .toInt(&ok);

        if (ok
            && value > 0
            && value <= 65535)
        {
            tcpPort =
                static_cast<quint16>(
                    value
                    );
        }
    }



    // =====================================================
    // ОТКРЫВАЕМ ARDUINO
    // =====================================================

    HANDLE serial =
        openArduinoPort(
            comPort
            );


    if (serial == INVALID_HANDLE_VALUE)
    {
        qCritical()
        << "Не удалось открыть"
        << comPort;

        qCritical()
            << "Код ошибки Windows:"
            << GetLastError();

        qCritical()
            << "Проверь, что Serial Monitor Arduino IDE закрыт.";

        return 1;
    }


    qInfo()
        << "Arduino открыта:"
        << comPort;

    qInfo()
        << "Скорость: 9600 baud";



    // =====================================================
    // TCP СЕРВЕР
    // =====================================================

    QTcpServer server;


    if (!server.listen(
            QHostAddress::AnyIPv4,
            tcpPort))
    {
        qCritical()
        << "Не удалось открыть TCP порт"
        << tcpPort;

        CloseHandle(serial);

        return 1;
    }


    qInfo()
        << "TCP сервер запущен";

    qInfo()
        << "Порт:"
        << tcpPort;


    qInfo()
        << "IPv4 адреса Windows:";


    const QList<QHostAddress> addresses =
        QNetworkInterface::allAddresses();


    for (const QHostAddress &address
         : addresses)
    {
        if (
            address.protocol()
                == QAbstractSocket::IPv4Protocol
            &&
            !address.isLoopback()
            )
        {
            qInfo()
            << address.toString();
        }
    }


    qInfo()
        << "Ожидание подключения Aurora...";



    // =====================================================
    // КЛИЕНТ AURORA
    // =====================================================

    QPointer<QTcpSocket>
        auroraClient;


    QObject::connect(
        &server,
        &QTcpServer::newConnection,
        [&]()
        {
            while (
                server.hasPendingConnections()
                )
            {
                QTcpSocket *newClient =
                    server
                        .nextPendingConnection();


                if (auroraClient)
                {
                    auroraClient
                        ->disconnectFromHost();
                }


                auroraClient =
                    newClient;


                qInfo()
                    << "Aurora подключилась:"
                    << newClient
                           ->peerAddress()
                           .toString();


                QObject::connect(
                    newClient,
                    &QTcpSocket::disconnected,
                    [&, newClient]()
                    {
                        qInfo()
                        << "Aurora отключилась";


                        if (
                            auroraClient
                            == newClient
                            )
                        {
                            auroraClient =
                                nullptr;
                        }


                        newClient
                            ->deleteLater();
                    }
                    );
            }
        }
        );



    // =====================================================
    // ЧТЕНИЕ COM13
    // =====================================================

    QByteArray serialBuffer;


    QTimer serialTimer;


    serialTimer.setInterval(
        20
        );


    QObject::connect(
        &serialTimer,
        &QTimer::timeout,
        [&]()
        {
            char data[256];

            DWORD bytesRead = 0;


            BOOL result =
                ReadFile(
                    serial,
                    data,
                    sizeof(data),
                    &bytesRead,
                    nullptr
                    );


            if (!result)
            {
                qWarning()
                << "Ошибка чтения COM";

                return;
            }


            if (bytesRead == 0)
            {
                return;
            }


            serialBuffer.append(
                data,
                static_cast<int>(
                    bytesRead
                    )
                );


            // =============================================
            // РАЗБИРАЕМ СТРОКИ ARDUINO
            // =============================================

            while (
                serialBuffer
                    .contains('\n')
                )
            {
                int position =
                    serialBuffer
                        .indexOf('\n');


                QByteArray line =
                    serialBuffer
                        .left(position)
                        .trimmed();


                serialBuffer.remove(
                    0,
                    position + 1
                    );


                if (line.isEmpty())
                {
                    continue;
                }


                bool ok = false;


                double temperature =
                    line.toDouble(&ok);


                if (!ok)
                {
                    qWarning()
                    << "Не число:"
                    << line;

                    continue;
                }


                // =========================================
                // ПРОВЕРКА LM35
                // =========================================

                if (
                    temperature < -55.0
                    ||
                    temperature > 150.0
                    )
                {
                    qWarning()
                    << "Некорректная температура:"
                    << temperature;

                    continue;
                }


                qInfo()
                    << "Температура:"
                    << temperature
                    << "C";


                // =========================================
                // ОТПРАВЛЯЕМ В AURORA
                // =========================================

                if (
                    auroraClient
                    &&
                    auroraClient->state()
                        ==
                        QAbstractSocket::ConnectedState
                    )
                {
                    QByteArray message =
                        QByteArray::number(
                            temperature,
                            'f',
                            1
                            );


                    message.append('\n');


                    auroraClient
                        ->write(
                            message
                            );


                    auroraClient
                        ->flush();
                }
            }
        }
        );


    serialTimer.start();



    // =====================================================
    // ЗАКРЫТИЕ COM
    // =====================================================

    QObject::connect(
        &app,
        &QCoreApplication::aboutToQuit,
        [&]()
        {
            if (
                serial
                != INVALID_HANDLE_VALUE
                )
            {
                CloseHandle(
                    serial
                    );
            }
        }
        );


    return app.exec();
}
