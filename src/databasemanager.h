#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>

class DatabaseManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sensorNumber
                   READ sensorNumber
                       NOTIFY dataChanged)

    Q_PROPERTY(QString sensorModel
                   READ sensorModel
                       NOTIFY dataChanged)

    Q_PROPERTY(QString description
                   READ description
                       NOTIFY dataChanged)

    Q_PROPERTY(QString unit
                   READ unit
                       NOTIFY dataChanged)

    Q_PROPERTY(double lastValue
                   READ lastValue
                       NOTIFY dataChanged)

    Q_PROPERTY(QString lastMeasurementTime
                   READ lastMeasurementTime
                       NOTIFY dataChanged)

    Q_PROPERTY(QVariantList temperatureLogs
                   READ temperatureLogs
                       NOTIFY temperatureLogsChanged)

public:
    explicit DatabaseManager(QObject *parent = nullptr);

    QString sensorNumber() const;
    QString sensorModel() const;
    QString description() const;
    QString unit() const;

    double lastValue() const;
    QString lastMeasurementTime() const;

    QVariantList temperatureLogs() const;

    // Позже сюда будем передавать температуру от Arduino
    Q_INVOKABLE bool addMeasurement(double value, int rawValue = -1);

signals:
    void dataChanged();
    void temperatureLogsChanged();

private:
    bool createTables();
    bool createMockData();
    void reload();

private:
    QSqlDatabase m_database;

    int m_sensorId = -1;

    QString m_sensorNumber;
    QString m_sensorModel;
    QString m_description;
    QString m_unit;

    double m_lastValue = 0.0;
    QString m_lastMeasurementTime;

    QVariantList m_temperatureLogs;
};

#endif
