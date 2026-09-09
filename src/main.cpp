#include <auroraapp.h>
#include <QtQuick>
#include <QQmlContext>

#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(
        Aurora::Application::application(argc, argv)
        );

    QScopedPointer<QQuickView> view(
        Aurora::Application::createView()
        );

    // Создаём объект, который работает с SQLite
    DatabaseManager databaseManager;

    // Передаём его в QML под именем databaseManager
    view->rootContext()->setContextProperty(
        "databaseManager",
        &databaseManager
        );

    view->setSource(
        Aurora::Application::pathTo(
            QStringLiteral("qml/untitled16.qml")
            )
        );

    view->show();

    return app->exec();
}
