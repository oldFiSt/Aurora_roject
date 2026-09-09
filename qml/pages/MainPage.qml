import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All


    function formatTime(value) {
        if (!value)
            return ""

        var date = new Date(value)

        return Qt.formatDateTime(
                    date,
                    "dd.MM.yyyy  hh:mm:ss"
                )
    }


    SilicaFlickable {
        anchors.fill: parent

        contentHeight: content.height


        Column {
            id: content

            width: parent.width

            spacing: Theme.paddingMedium


            PageHeader {
                title: "Контроль температуры"
            }


            // ============================
            // ДАТЧИК
            // ============================

            SectionHeader {
                text: "Информация о датчике"
            }


            Label {
                x: Theme.horizontalPageMargin

                width:
                    parent.width
                    - 2 * Theme.horizontalPageMargin

                text:
                    "Номер датчика: "
                    + databaseManager.sensorNumber

                font.pixelSize:
                    Theme.fontSizeLarge
            }


            Label {
                x: Theme.horizontalPageMargin

                text:
                    "Модель: "
                    + databaseManager.sensorModel
            }


            Label {
                x: Theme.horizontalPageMargin

                width:
                    parent.width
                    - 2 * Theme.horizontalPageMargin

                wrapMode: Text.WordWrap

                text:
                    "Описание: "
                    + databaseManager.description
            }


            // ============================
            // ТЕКУЩАЯ ТЕМПЕРАТУРА
            // ============================

            SectionHeader {
                text: "Текущая температура"
            }


            Label {
                anchors.horizontalCenter:
                    parent.horizontalCenter

                text:
                    databaseManager.lastValue.toFixed(1)
                    + " "
                    + databaseManager.unit

                font.pixelSize:
                    Theme.fontSizeHuge
            }


            Label {
                anchors.horizontalCenter:
                    parent.horizontalCenter

                text:
                    "Последнее измерение: "
                    + formatTime(
                        databaseManager.lastMeasurementTime
                    )

                color:
                    Theme.secondaryColor
            }


            // ============================
            // ВРЕМЕННАЯ КНОПКА ДЛЯ ТЕСТА
            // ============================

            SectionHeader {
                text: "Тест изменения температуры"
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingMedium

                Button {
                    text: "-0.3"

                    onClicked: {
                        databaseManager.addMeasurement(
                            databaseManager.lastValue - 0.3
                        )
                    }
                }

                Button {
                    text: "-0.1"

                    onClicked: {
                        databaseManager.addMeasurement(
                            databaseManager.lastValue - 0.1
                        )
                    }
                }

                Button {
                    text: "+0.1"

                    onClicked: {
                        databaseManager.addMeasurement(
                            databaseManager.lastValue + 0.1
                        )
                    }
                }

                Button {
                    text: "+0.3"

                    onClicked: {
                        databaseManager.addMeasurement(
                            databaseManager.lastValue + 0.3
                        )
                    }
                }
            }


            // ============================
            // ЛОГИ ТЕМПЕРАТУРЫ
            // ============================

            SectionHeader {
                text: "История изменения температуры"
            }


            Repeater {
                model:
                    databaseManager.temperatureLogs


                delegate: Column {
                    width: parent.width

                    spacing:
                        Theme.paddingSmall


                    Label {
                        x: Theme.horizontalPageMargin

                        width:
                            parent.width
                            - 2 * Theme.horizontalPageMargin

                        text:
                            Number(modelData.oldValue)
                            .toFixed(1)
                            + " °C  →  "
                            + Number(modelData.newValue)
                            .toFixed(1)
                            + " °C"

                        font.pixelSize:
                            Theme.fontSizeLarge
                    }


                    Label {
                        x: Theme.horizontalPageMargin

                        text:
                            "Изменение: "
                            + (modelData.delta >= 0
                               ? "+"
                               : "")
                            + Number(modelData.delta)
                              .toFixed(1)
                            + " °C"

                        color:
                            Theme.secondaryHighlightColor
                    }


                    Label {
                        x: Theme.horizontalPageMargin

                        text:
                            formatTime(modelData.time)

                        color:
                            Theme.secondaryColor

                        font.pixelSize:
                            Theme.fontSizeSmall
                    }


                    Separator {
                        width: parent.width
                    }
                }
            }


            Item {
                width: 1
                height: Theme.paddingLarge
            }
        }
    }
}
