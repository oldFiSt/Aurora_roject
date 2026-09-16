#include <auroraapp.h>
#include <QtQuick>
#include <QQmlContext>

#include "databasemanager.h"
#include "temperaturereceiver.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(
        Aurora::Application::application(argc, argv));

    QScopedPointer<QQuickView> view(
        Aurora::Application::createView());

    DatabaseManager databaseManager;

    view->rootContext()->setContextProperty(
        "databaseManager",
        &databaseManager);

    TemperatureReceiver temperatureReceiver;

    view->rootContext()->setContextProperty(
        "temperatureReceiver",
        &temperatureReceiver);

    QObject::connect(
        &temperatureReceiver,
        &TemperatureReceiver::temperatureReceived,
        &databaseManager,
        [&databaseManager](double temperature)
        {
            databaseManager.addMeasurement(
                temperature);
        });

    view->setSource(
        Aurora::Application::pathTo(
            QStringLiteral("qml/untitled16.qml")));

    view->show();

    return app->exec();
}
