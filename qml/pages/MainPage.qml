import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    // =========================================================
    // АДАПТИВНЫЙ МАСШТАБ
    // =========================================================

    property real uiScale:
        Math.max(
            1.15,
            Math.min(width / 500.0, 1.55)
        )

    property bool wideMode:
        width > height && width >= 900

    property int sideMargin:
        Math.round(24 * uiScale)

    property int cardRadius:
        Math.round(24 * uiScale)

    property int sectionSpacing:
        Math.round(20 * uiScale)

    // =========================================================
    // ЦВЕТА
    // =========================================================

    property color backgroundColor: "#F2F6F3"
    property color cardColor: "#FFFFFF"

    property color textMain: "#17201A"
    property color textSecondary: "#6E7871"

    property color green: "#25A447"
    property color greenDark: "#157D36"

    property color greenSoft: "#E9F7ED"
    property color greenLight: "#D5F0DD"

    property color borderColor: "#DCE6DF"
    property color redColor: "#E54747"

    property int logLimit: wideMode ? 8 : 5

    // =========================================================
    // ПОДКЛЮЧЕНИЕ К C++ МОСТУ НА WINDOWS
    // =========================================================

    // Для QEMU обычно можно начать с 10.0.2.2.
    // Если соединения нет, укажи здесь IPv4-адрес Windows-компьютера.
    property string bridgeHost: "10.0.2.2"
    property int bridgePort: 5555
    property string windowsComPort: "COM13"


    // =========================================================
    // ФОРМАТИРОВАНИЕ ВРЕМЕНИ
    // =========================================================

    function formatTime(value) {

        if (!value)
            return "Нет данных"

        var date = new Date(value)

        return Qt.formatDateTime(
                    date,
                    "dd.MM.yyyy  hh:mm:ss"
                )
    }


    // Подключаемся к Windows C++ bridge.
    // TemperatureReceiver сам будет переподключаться при обрыве.
    Component.onCompleted: {
        temperatureReceiver.connectToServer(
                    page.bridgeHost,
                    page.bridgePort
                    )
    }


    // =========================================================
    // ФОН
    // =========================================================

    Rectangle {
        anchors.fill: parent
        color: page.backgroundColor
    }


    // =========================================================
    // ПРОКРУТКА
    // =========================================================

    SilicaFlickable {
        id: flick

        anchors.fill: parent

        contentHeight:
            mainColumn.height
            + Math.round(50 * page.uiScale)

        clip: true


        Column {
            id: mainColumn

            width: parent.width

            spacing:
                Math.round(24 * page.uiScale)


            // =================================================
            // HEADER
            // =================================================

            Rectangle {
                width: parent.width

                height:
                    Math.round(205 * page.uiScale)

                color: "#FFFFFF"


                Column {
                    anchors.centerIn: parent

                    width:
                        parent.width
                        - 2 * page.sideMargin

                    spacing:
                        Math.round(12 * page.uiScale)


                    // =========================================
                    // СОВРЕМЕННЫЙ ЛОГОТИП
                    // =========================================

                    Row {
                        anchors.horizontalCenter:
                            parent.horizontalCenter

                        spacing:
                            Math.round(14 * page.uiScale)


                        // =====================================
                        // ЗНАК
                        // =====================================

                        Rectangle {
                            width:
                                Math.round(58 * page.uiScale)

                            height:
                                width

                            radius:
                                Math.round(15 * page.uiScale)

                            color:
                                page.green


                            // верх буквы Т

                            Rectangle {
                                width:
                                    parent.width * 0.58

                                height:
                                    Math.round(9 * page.uiScale)

                                radius:
                                    height / 2

                                anchors.horizontalCenter:
                                    parent.horizontalCenter

                                y:
                                    Math.round(15 * page.uiScale)

                                color:
                                    "#FFFFFF"
                            }


                            // ножка буквы Т

                            Rectangle {
                                width:
                                    Math.round(9 * page.uiScale)

                                height:
                                    parent.height * 0.48

                                radius:
                                    width / 2

                                anchors.horizontalCenter:
                                    parent.horizontalCenter

                                y:
                                    Math.round(18 * page.uiScale)

                                color:
                                    "#FFFFFF"
                            }
                        }


                        Column {
                            anchors.verticalCenter:
                                parent.verticalCenter

                            spacing:
                                Math.round(1 * page.uiScale)


                            Label {
                                text: "ТПУ"

                                color:
                                    page.textMain

                                font.pixelSize:
                                    Math.round(28 * page.uiScale)

                                font.bold: true
                            }


                            Label {
                                text:
                                    "ТОМСКИЙ ПОЛИТЕХНИЧЕСКИЙ"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(11 * page.uiScale)

                                font.bold: true
                            }


                            Label {
                                text:
                                    "УНИВЕРСИТЕТ"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(11 * page.uiScale)

                                font.bold: true
                            }
                        }
                    }


                    Item {
                        width: 1

                        height:
                            Math.round(8 * page.uiScale)
                    }


                    Label {
                        anchors.horizontalCenter:
                            parent.horizontalCenter

                        text:
                            "Мониторинг температуры"

                        color:
                            page.textMain

                        font.pixelSize:
                            Math.round(29 * page.uiScale)

                        font.bold: true
                    }


                    Label {
                        anchors.horizontalCenter:
                            parent.horizontalCenter

                        text:
                            "Система контроля датчика LM35"

                        color:
                            page.textSecondary

                        font.pixelSize:
                            Math.round(17 * page.uiScale)
                    }
                }


                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    height: 1

                    color:
                        page.borderColor
                }
            }



            // =================================================
            // ОСНОВНОЙ КОНТЕНТ
            // =================================================

            Column {
                width:
                    Math.min(
                        parent.width
                        - 2 * page.sideMargin,
                        1150
                    )

                anchors.horizontalCenter:
                    parent.horizontalCenter

                spacing:
                    page.sectionSpacing



                // =====================================================
                // ИНФОРМАЦИЯ О ДАТЧИКЕ
                // =====================================================

                Rectangle {
                    width: parent.width

                    height:
                        sensorContent.height
                        + Math.round(50 * page.uiScale)

                    radius:
                        page.cardRadius

                    color:
                        page.cardColor

                    border.width: 1

                    border.color:
                        page.borderColor


                    Column {
                        id: sensorContent

                        width:
                            parent.width
                            - Math.round(48 * page.uiScale)

                        anchors.centerIn:
                            parent

                        spacing:
                            Math.round(15 * page.uiScale)


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                "Информация о датчике"

                            color:
                                page.textMain

                            font.pixelSize:
                                Math.round(23 * page.uiScale)

                            font.bold: true
                        }


                        Rectangle {
                            width: parent.width
                            height: 1
                            color: page.borderColor
                        }


                        Row {
                            width: parent.width


                            Label {
                                width:
                                    parent.width * 0.45

                                text:
                                    "Номер датчика"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }


                            Label {
                                width:
                                    parent.width * 0.55

                                text:
                                    databaseManager.sensorNumber

                                color:
                                    page.textMain

                                horizontalAlignment:
                                    Text.AlignRight

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)

                                font.bold: true
                            }
                        }


                        Row {
                            width: parent.width


                            Label {
                                width:
                                    parent.width * 0.45

                                text:
                                    "Модель"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }


                            Label {
                                width:
                                    parent.width * 0.55

                                text:
                                    databaseManager.sensorModel

                                color:
                                    page.textMain

                                horizontalAlignment:
                                    Text.AlignRight

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)

                                font.bold: true
                            }
                        }


                        Row {
                            width: parent.width


                            Label {
                                width:
                                    parent.width * 0.35

                                text:
                                    "Описание"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }


                            Label {
                                width:
                                    parent.width * 0.65

                                text:
                                    databaseManager.description

                                color:
                                    page.textMain

                                horizontalAlignment:
                                    Text.AlignRight

                                wrapMode:
                                    Text.WordWrap

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }
                        }
                    }
                }



                // =====================================================
                // ГЛАВНАЯ КАРТОЧКА ТЕМПЕРАТУРЫ
                // =====================================================

                Rectangle {
                    width: parent.width

                    height:
                        Math.round(330 * page.uiScale)

                    radius:
                        Math.round(28 * page.uiScale)

                    color:
                        page.greenSoft

                    border.width: 1

                    border.color:
                        "#CDE8D5"


                    Column {
                        width:
                            parent.width
                            - Math.round(40 * page.uiScale)

                        anchors.centerIn:
                            parent

                        spacing:
                            Math.round(10 * page.uiScale)


                        // =============================================
                        // ИКОНКА ТЕРМОМЕТРА
                        // =============================================

                        Rectangle {
                            width:
                                Math.round(94 * page.uiScale)

                            height:
                                width

                            radius:
                                width / 2

                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            color:
                                page.greenLight


                            Item {
                                width:
                                    Math.round(38 * page.uiScale)

                                height:
                                    Math.round(62 * page.uiScale)

                                anchors.centerIn:
                                    parent


                                Rectangle {
                                    width:
                                        Math.round(15 * page.uiScale)

                                    height:
                                        Math.round(43 * page.uiScale)

                                    radius:
                                        width / 2

                                    anchors.horizontalCenter:
                                        parent.horizontalCenter

                                    color:
                                        page.greenDark
                                }


                                Rectangle {
                                    width:
                                        Math.round(31 * page.uiScale)

                                    height:
                                        width

                                    radius:
                                        width / 2

                                    anchors.horizontalCenter:
                                        parent.horizontalCenter

                                    anchors.bottom:
                                        parent.bottom

                                    color:
                                        page.greenDark
                                }


                                Rectangle {
                                    width:
                                        Math.round(6 * page.uiScale)

                                    height:
                                        Math.round(36 * page.uiScale)

                                    radius:
                                        width / 2

                                    anchors.horizontalCenter:
                                        parent.horizontalCenter

                                    y:
                                        Math.round(4 * page.uiScale)

                                    color:
                                        "#FFFFFF"
                                }
                            }
                        }


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                "Текущая температура"

                            color:
                                page.textSecondary

                            font.pixelSize:
                                Math.round(19 * page.uiScale)
                        }


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                databaseManager
                                .lastValue
                                .toFixed(1)
                                + " "
                                + databaseManager.unit

                            color:
                                page.textMain

                            font.pixelSize:
                                Math.round(58 * page.uiScale)

                            font.bold: true
                        }


                        Rectangle {
                            width:
                                parent.width * 0.72

                            height: 1

                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            color:
                                "#C7DFCE"
                        }


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                "Последнее обновление"

                            color:
                                page.textSecondary

                            font.pixelSize:
                                Math.round(14 * page.uiScale)
                        }


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                page.formatTime(
                                    databaseManager.lastMeasurementTime
                                )

                            color:
                                page.textMain

                            font.pixelSize:
                                Math.round(16 * page.uiScale)

                            font.bold: true
                        }


                        Row {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            spacing:
                                Math.round(8 * page.uiScale)


                            Rectangle {
                                width:
                                    Math.round(12 * page.uiScale)

                                height:
                                    width

                                radius:
                                    width / 2

                                anchors.verticalCenter:
                                    parent.verticalCenter

                                color:
                                    temperatureReceiver.connected
                                    ? page.green
                                    : "#A0A8A3"
                            }


                            Label {
                                text:
                                    temperatureReceiver.connected
                                    ? "Датчик подключен"
                                    : "Нет соединения с Arduino"

                                color:
                                    temperatureReceiver.connected
                                    ? page.greenDark
                                    : page.textSecondary

                                font.pixelSize:
                                    Math.round(16 * page.uiScale)

                                font.bold: true
                            }
                        }
                    }
                }



                // =====================================================
                // ПОТОК ДАННЫХ
                // =====================================================

                Rectangle {
                    width: parent.width

                    height:
                        streamContent.height
                        + Math.round(52 * page.uiScale)

                    radius:
                        page.cardRadius

                    color:
                        page.cardColor

                    border.width: 1

                    border.color:
                        page.borderColor


                    Column {
                        id: streamContent

                        width:
                            parent.width
                            - Math.round(48 * page.uiScale)

                        anchors.centerIn:
                            parent

                        spacing:
                            Math.round(15 * page.uiScale)


                        Row {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            spacing:
                                Math.round(10 * page.uiScale)


                            Rectangle {
                                width:
                                    Math.round(14 * page.uiScale)

                                height:
                                    width

                                radius:
                                    width / 2

                                anchors.verticalCenter:
                                    parent.verticalCenter

                                color:
                                    page.green
                            }


                            Label {
                                text:
                                    "Поток данных"

                                color:
                                    page.textMain

                                font.pixelSize:
                                    Math.round(23 * page.uiScale)

                                font.bold: true
                            }
                        }


                        Rectangle {
                            width: parent.width

                            height: 1

                            color:
                                page.borderColor
                        }


                        Row {
                            width: parent.width


                            Label {
                                width:
                                    parent.width * 0.45

                                text:
                                    "Источник"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }


                            Label {
                                width:
                                    parent.width * 0.55

                                text:
                                    "Arduino Uno / LM35"

                                horizontalAlignment:
                                    Text.AlignRight

                                color:
                                    page.textMain

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)

                                font.bold: true
                            }
                        }


                        Row {
                            width: parent.width


                            Label {
                                width:
                                    parent.width * 0.45

                                text:
                                    "COM-порт"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }


                            Label {
                                width:
                                    parent.width * 0.55

                                text:
                                    page.windowsComPort + " (Windows)"

                                color:
                                    page.textMain

                                horizontalAlignment:
                                    Text.AlignRight

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)

                                font.bold: true
                            }
                        }


                        Row {
                            width: parent.width


                            Label {
                                width:
                                    parent.width * 0.45

                                text:
                                    "Обновление"

                                color:
                                    page.textSecondary

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }


                            Label {
                                width:
                                    parent.width * 0.55

                                text:
                                    "1 сек"

                                color:
                                    page.textMain

                                horizontalAlignment:
                                    Text.AlignRight

                                font.pixelSize:
                                    Math.round(17 * page.uiScale)
                            }
                        }
                    }
                }



                // =====================================================
                // ЖУРНАЛ
                // =====================================================

                Rectangle {
                    width:
                        parent.width

                    height:
                        journalContent.height
                        + Math.round(50 * page.uiScale)

                    radius:
                        page.cardRadius

                    color:
                        page.cardColor

                    border.width: 1

                    border.color:
                        page.borderColor


                    Column {
                        id: journalContent

                        width:
                            parent.width
                            - Math.round(48 * page.uiScale)

                        anchors.centerIn:
                            parent

                        spacing:
                            Math.round(14 * page.uiScale)


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                "История изменений"

                            color:
                                page.textMain

                            font.pixelSize:
                                Math.round(23 * page.uiScale)

                            font.bold: true
                        }


                        Label {
                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                "Последних записей: "
                                + Math.min(
                                    databaseManager.temperatureLogs.length,
                                    page.logLimit
                                )

                            color:
                                page.textSecondary

                            font.pixelSize:
                                Math.round(14 * page.uiScale)
                        }


                        Rectangle {
                            width: parent.width

                            height: 1

                            color:
                                page.borderColor
                        }


                        Repeater {
                            model:
                                Math.min(
                                    databaseManager.temperatureLogs.length,
                                    page.logLimit
                                )


                            delegate: Rectangle {
                                width:
                                    journalContent.width

                                height:
                                    Math.round(84 * page.uiScale)

                                radius:
                                    Math.round(16 * page.uiScale)

                                color:
                                    "#F7FAF8"


                                Column {
                                    width:
                                        parent.width
                                        - Math.round(30 * page.uiScale)

                                    anchors.centerIn:
                                        parent

                                    spacing:
                                        Math.round(7 * page.uiScale)


                                    Row {
                                        width: parent.width


                                        Label {
                                            width:
                                                parent.width * 0.70

                                            text:
                                                Number(
                                                    databaseManager
                                                    .temperatureLogs[index]
                                                    .oldValue
                                                ).toFixed(1)
                                                + " °C   →   "
                                                + Number(
                                                    databaseManager
                                                    .temperatureLogs[index]
                                                    .newValue
                                                ).toFixed(1)
                                                + " °C"

                                            color:
                                                page.textMain

                                            font.pixelSize:
                                                Math.round(17 * page.uiScale)

                                            font.bold: true
                                        }


                                        Label {
                                            width:
                                                parent.width * 0.30

                                            property real deltaValue:
                                                Number(
                                                    databaseManager
                                                    .temperatureLogs[index]
                                                    .delta
                                                )

                                            text:
                                                (
                                                    deltaValue >= 0
                                                    ? "+"
                                                    : ""
                                                )
                                                + deltaValue.toFixed(1)
                                                + " °C"

                                            color:
                                                deltaValue >= 0
                                                ? page.green
                                                : page.redColor

                                            horizontalAlignment:
                                                Text.AlignRight

                                            font.pixelSize:
                                                Math.round(17 * page.uiScale)

                                            font.bold: true
                                        }
                                    }


                                    Label {
                                        text:
                                            page.formatTime(
                                                databaseManager
                                                .temperatureLogs[index]
                                                .time
                                            )

                                        color:
                                            page.textSecondary

                                        font.pixelSize:
                                            Math.round(14 * page.uiScale)
                                    }
                                }
                            }
                        }


                        Label {
                            visible:
                                databaseManager
                                .temperatureLogs
                                .length === 0

                            anchors.horizontalCenter:
                                parent.horizontalCenter

                            text:
                                "Журнал пока пуст"

                            color:
                                page.textSecondary

                            font.pixelSize:
                                Math.round(17 * page.uiScale)
                        }
                    }
                }



                Item {
                    width: 1

                    height:
                        Math.round(30 * page.uiScale)
                }
            }
        }
    }
}
