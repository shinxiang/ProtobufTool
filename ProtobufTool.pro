QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.cpp \
    main.cpp \
    pb_tool.cpp \
    protobuftoolmain.cpp

HEADERS += \
    config.h \
    pb_tool.h \
    protobuftoolmain.h

FORMS += \
    protobuftoolmain.ui

TRANSLATIONS += \
    ProtobufTool_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_FILE=icon/logo.rc

INCLUDEPATH += D:\software\protobuf-cpp-3.13.0\protobuf-3.13.0\src
LIBS += -L D:\software\protobuf-cpp-3.13.0\build -lprotobuf

include(json/json.pri)

DISTFILES += \
    icon/logo.ico \
    icon/logo.rc \
    setting.ini \
    test.proto
