#include "databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QVariantMap>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    QString dataPath =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);

    QDir().mkpath(dataPath);

    /*
     * Используем новое имя БД,
     * чтобы старая БД с coefficient нам не мешала.
     */
    QString databasePath =
        dataPath + "/sensors_v2.db";

    qDebug() << "Database path:" << databasePath;

    m_database =
        QSqlDatabase::addDatabase(
            "QSQLITE",
            "sensor_database"
            );

    m_database.setDatabaseName(databasePath);

    if (!m_database.open())
    {
        qDebug() << "Database error:"
                 << m_database.lastError().text();
        return;
    }

    QSqlQuery pragma(m_database);
    pragma.exec("PRAGMA foreign_keys = ON");

    createTables();
    createMockData();
    reload();
}


bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // -------------------------
    // ДАТЧИКИ
    // -------------------------

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS sensors ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "sensor_number TEXT UNIQUE NOT NULL,"
            "model TEXT,"
            "description TEXT,"
            "unit TEXT,"
            "created_at TEXT"
            ")"
            ))
    {
        qDebug() << "sensors:"
                 << query.lastError().text();
        return false;
    }


    // -------------------------
    // ВСЕ ИЗМЕРЕНИЯ
    // -------------------------

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS measurements ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "sensor_id INTEGER NOT NULL,"
            "measured_at TEXT NOT NULL,"
            "value REAL NOT NULL,"
            "raw_value INTEGER,"
            "FOREIGN KEY(sensor_id) REFERENCES sensors(id)"
            ")"
            ))
    {
        qDebug() << "measurements:"
                 << query.lastError().text();
        return false;
    }


    // -------------------------
    // ЛОГИ ИЗМЕНЕНИЯ ТЕМПЕРАТУРЫ
    // -------------------------

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS temperature_logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "sensor_id INTEGER NOT NULL,"
            "changed_at TEXT NOT NULL,"
            "old_temperature REAL NOT NULL,"
            "new_temperature REAL NOT NULL,"
            "FOREIGN KEY(sensor_id) REFERENCES sensors(id)"
            ")"
            ))
    {
        qDebug() << "temperature_logs:"
                 << query.lastError().text();
        return false;
    }

    return true;
}


bool DatabaseManager::createMockData()
{
    QSqlQuery query(m_database);

    // Создаём датчик LM35
    query.prepare(
        "INSERT OR IGNORE INTO sensors "
        "(sensor_number, model, description, unit, created_at) "
        "VALUES (?, ?, ?, ?, ?)"
        );

    query.addBindValue("T-001");
    query.addBindValue("LM35");
    query.addBindValue("Датчик температуры платы №1");
    query.addBindValue("°C");
    query.addBindValue(
        QDateTime::currentDateTime().toString(Qt::ISODate)
        );

    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        return false;
    }


    // Получаем ID датчика
    query.prepare(
        "SELECT id "
        "FROM sensors "
        "WHERE sensor_number = ?"
        );

    query.addBindValue("T-001");

    if (!query.exec())
        return false;

    if (query.next())
        m_sensorId = query.value(0).toInt();


    // Проверяем наличие моковых измерений
    query.prepare(
        "SELECT COUNT(*) "
        "FROM measurements "
        "WHERE sensor_id = ?"
        );

    query.addBindValue(m_sensorId);
    query.exec();

    int count = 0;

    if (query.next())
        count = query.value(0).toInt();


    // Если БД новая — создаём тестовые данные
    if (count == 0)
    {
        QDateTime now =
            QDateTime::currentDateTime();

        struct MockMeasurement
        {
            int secondsAgo;
            double value;
            int rawValue;
        };

        MockMeasurement values[] =
            {
                { 60, 24.5, 50 },
                { 40, 24.7, 51 },
                { 20, 24.6, 50 },
                {  0, 24.9, 51 }
            };

        double previousValue = 0.0;
        bool hasPrevious = false;

        for (const MockMeasurement &item : values)
        {
            QString time =
                now.addSecs(-item.secondsAgo)
                    .toString(Qt::ISODate);

            // Записываем измерение
            QSqlQuery insert(m_database);

            insert.prepare(
                "INSERT INTO measurements "
                "(sensor_id, measured_at, value, raw_value) "
                "VALUES (?, ?, ?, ?)"
                );

            insert.addBindValue(m_sensorId);
            insert.addBindValue(time);
            insert.addBindValue(item.value);
            insert.addBindValue(item.rawValue);

            if (!insert.exec())
            {
                qDebug() << insert.lastError().text();
                return false;
            }


            // Если температура изменилась —
            // создаём лог
            if (hasPrevious &&
                !qFuzzyCompare(previousValue + 1.0,
                               item.value + 1.0))
            {
                QSqlQuery log(m_database);

                log.prepare(
                    "INSERT INTO temperature_logs "
                    "(sensor_id, changed_at, "
                    "old_temperature, new_temperature) "
                    "VALUES (?, ?, ?, ?)"
                    );

                log.addBindValue(m_sensorId);
                log.addBindValue(time);
                log.addBindValue(previousValue);
                log.addBindValue(item.value);

                if (!log.exec())
                {
                    qDebug() << log.lastError().text();
                    return false;
                }
            }

            previousValue = item.value;
            hasPrevious = true;
        }
    }

    return true;
}


void DatabaseManager::reload()
{
    QSqlQuery query(m_database);

    // -------------------------
    // ИНФОРМАЦИЯ О ДАТЧИКЕ
    // -------------------------

    query.prepare(
        "SELECT id, sensor_number, model, "
        "description, unit "
        "FROM sensors "
        "WHERE sensor_number = ?"
        );

    query.addBindValue("T-001");

    if (query.exec() && query.next())
    {
        m_sensorId =
            query.value(0).toInt();

        m_sensorNumber =
            query.value(1).toString();

        m_sensorModel =
            query.value(2).toString();

        m_description =
            query.value(3).toString();

        m_unit =
            query.value(4).toString();
    }


    // -------------------------
    // ПОСЛЕДНЯЯ ТЕМПЕРАТУРА
    // -------------------------

    query.prepare(
        "SELECT value, measured_at "
        "FROM measurements "
        "WHERE sensor_id = ? "
        "ORDER BY id DESC "
        "LIMIT 1"
        );

    query.addBindValue(m_sensorId);

    if (query.exec() && query.next())
    {
        m_lastValue =
            query.value(0).toDouble();

        m_lastMeasurementTime =
            query.value(1).toString();
    }


    // -------------------------
    // ИСТОРИЯ ИЗМЕНЕНИЙ
    // -------------------------

    m_temperatureLogs.clear();

    query.prepare(
        "SELECT changed_at, "
        "old_temperature, "
        "new_temperature "
        "FROM temperature_logs "
        "WHERE sensor_id = ? "
        "ORDER BY id DESC "
        "LIMIT 50"
        );

    query.addBindValue(m_sensorId);

    if (query.exec())
    {
        while (query.next())
        {
            QVariantMap row;

            row["time"] =
                query.value(0).toString();

            row["oldValue"] =
                query.value(1).toDouble();

            row["newValue"] =
                query.value(2).toDouble();

            row["delta"] =
                query.value(2).toDouble()
                - query.value(1).toDouble();

            m_temperatureLogs.append(row);
        }
    }

    emit dataChanged();
    emit temperatureLogsChanged();
}


bool DatabaseManager::addMeasurement(
    double value,
    int rawValue)
{
    const double LOG_THRESHOLD = 0.2;
    const double EPSILON = 0.000001;

    /*
     * referenceValue — температура, относительно
     * которой определяем значимое изменение.
     *
     * Сначала ищем последнее записанное
     * изменение температуры.
     */
    double referenceValue = 0.0;
    bool hasReference = false;

    QSqlQuery referenceQuery(m_database);

    referenceQuery.prepare(
        "SELECT new_temperature "
        "FROM temperature_logs "
        "WHERE sensor_id = ? "
        "ORDER BY id DESC "
        "LIMIT 1"
        );

    referenceQuery.addBindValue(m_sensorId);

    if (referenceQuery.exec() &&
        referenceQuery.next())
    {
        referenceValue =
            referenceQuery.value(0).toDouble();

        hasReference = true;
    }


    /*
     * Если логов ещё вообще нет,
     * точкой отсчёта становится самое
     * первое измерение датчика.
     */
    if (!hasReference)
    {
        QSqlQuery firstMeasurement(m_database);

        firstMeasurement.prepare(
            "SELECT value "
            "FROM measurements "
            "WHERE sensor_id = ? "
            "ORDER BY id ASC "
            "LIMIT 1"
            );

        firstMeasurement.addBindValue(m_sensorId);

        if (firstMeasurement.exec() &&
            firstMeasurement.next())
        {
            referenceValue =
                firstMeasurement.value(0).toDouble();

            hasReference = true;
        }
    }


    QString currentTime =
        QDateTime::currentDateTime()
            .toString(Qt::ISODate);


    if (!m_database.transaction())
        return false;


    // =====================================
    // СОХРАНЯЕМ ЛЮБОЕ ИЗМЕРЕНИЕ
    // =====================================

    QSqlQuery insert(m_database);

    insert.prepare(
        "INSERT INTO measurements "
        "(sensor_id, measured_at, value, raw_value) "
        "VALUES (?, ?, ?, ?)"
        );

    insert.addBindValue(m_sensorId);
    insert.addBindValue(currentTime);
    insert.addBindValue(value);
    insert.addBindValue(rawValue);

    if (!insert.exec())
    {
        qDebug()
        << "Measurement insert error:"
        << insert.lastError().text();

        m_database.rollback();
        return false;
    }


    // =====================================
    // ПРОВЕРЯЕМ НАКОПЛЕННОЕ ИЗМЕНЕНИЕ
    // =====================================

    if (hasReference)
    {
        double difference =
            qAbs(value - referenceValue);

        /*
         * EPSILON нужен из-за особенностей
         * double:
         *
         * 24.5 - 24.3 иногда получается
         * не ровно 0.2, а 0.199999999...
         */
        if (difference + EPSILON >= LOG_THRESHOLD)
        {
            QSqlQuery log(m_database);

            log.prepare(
                "INSERT INTO temperature_logs "
                "(sensor_id, changed_at, "
                "old_temperature, new_temperature) "
                "VALUES (?, ?, ?, ?)"
                );

            log.addBindValue(m_sensorId);
            log.addBindValue(currentTime);

            /*
             * В old_temperature пишем именно
             * последнюю ЗАФИКСИРОВАННУЮ температуру,
             * а не предыдущее измерение.
             */
            log.addBindValue(referenceValue);
            log.addBindValue(value);

            if (!log.exec())
            {
                qDebug()
                << "Temperature log error:"
                << log.lastError().text();

                m_database.rollback();
                return false;
            }
        }
    }


    if (!m_database.commit())
        return false;


    reload();

    return true;
}


QString DatabaseManager::sensorNumber() const
{
    return m_sensorNumber;
}


QString DatabaseManager::sensorModel() const
{
    return m_sensorModel;
}


QString DatabaseManager::description() const
{
    return m_description;
}


QString DatabaseManager::unit() const
{
    return m_unit;
}


double DatabaseManager::lastValue() const
{
    return m_lastValue;
}


QString DatabaseManager::lastMeasurementTime() const
{
    return m_lastMeasurementTime;
}


QVariantList DatabaseManager::temperatureLogs() const
{
    return m_temperatureLogs;
}
