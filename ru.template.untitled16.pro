TARGET = ru.template.untitled16

CONFIG += \
    auroraapp

QT += sql network

SOURCES += \
    src/databasemanager.cpp \
    src/temperaturereceiver.cpp \
    src/main.cpp

HEADERS += \
    src/databasemanager.h \
    src/temperaturereceiver.h

DISTFILES += \
    rpm/ru.template.untitled16.spec

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += auroraapp_i18n

TRANSLATIONS += \
    translations/ru.template.untitled16.ts \
    translations/ru.template.untitled16-ru.ts
