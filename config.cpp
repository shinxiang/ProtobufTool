#include "config.h"
#include <QSettings>

#define PROTO_GROUP "PROTO"

Config::Config()
{

}

Config::~Config()
{

}

// 读配置文件
void Config::read_config(std::string key, std::string& value) {
    QSettings settings("setting.ini", QSettings::IniFormat);
    settings.beginGroup(PROTO_GROUP);
    value = settings.value(QString::fromStdString(key)).toString().toStdString();
    settings.endGroup();
}

// 写配置文件
void Config::write_config(std::string key, std::string value) {
    QSettings settings("setting.ini", QSettings::IniFormat);
    settings.beginGroup(PROTO_GROUP);
    settings.setValue(QString::fromStdString(key), QString::fromStdString(value));
    settings.endGroup();
}
