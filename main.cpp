#include "protobuftoolmain.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont f = a.font();
    f.setFamily("Tahoma"); // 修改字体
    a.setFont(f);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ProtobufTool_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    ProtobufToolMain w;
    w.setWindowTitle(QStringLiteral("Protobuf 编解码工具 V1.1"));
    w.show();
    return a.exec();
}
