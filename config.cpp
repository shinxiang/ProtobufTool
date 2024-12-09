#include "config.h"
#include <QSettings>

#define PROTO_GROUP "PROTO"

// 配置文件
#define SETTING_FILE "setting.ini"

Config::Config()
{
    m_protoFileName = "";
    m_jsonPrintAll = 0;
    m_headLength = 0;
    m_tailLength = 0;
}

Config::Config(int jsonPrintAll, int headLength, int tailLength)
{
    m_jsonPrintAll = jsonPrintAll;
    m_headLength = headLength;
    m_tailLength = tailLength;
}

Config::~Config()
{

}

// 加载配置
void Config::load_config() {
    this->read_value("filename", this->m_protoFileName);
    this->read_value("jsonprint", this->m_jsonPrintAll);
    this->read_value("headlength", this->m_headLength);
    this->read_value("taillength", this->m_tailLength);
}

// 保存配置
void Config::save_config() {
    this->write_value("filename", this->m_protoFileName);
    this->write_value("jsonprint", std::to_string(this->m_jsonPrintAll));
    this->write_value("headlength", std::to_string(this->m_headLength));
    this->write_value("taillength", std::to_string(this->m_tailLength));
}

// 读配置文件
void Config::read_value(std::string key, std::string& value) {
    QSettings settings(SETTING_FILE, QSettings::IniFormat);
    settings.beginGroup(PROTO_GROUP);
    value = settings.value(QString::fromStdString(key)).toString().toStdString();
    settings.endGroup();
}

// 写配置文件
void Config::write_value(std::string key, std::string value) {
    QSettings settings(SETTING_FILE, QSettings::IniFormat);
    settings.beginGroup(PROTO_GROUP);
    settings.setValue(QString::fromStdString(key), QString::fromStdString(value));
    settings.endGroup();
}

void Config::read_value(std::string key, int& value) {
    QSettings settings(SETTING_FILE, QSettings::IniFormat);
    settings.beginGroup(PROTO_GROUP);
    value = settings.value(QString::fromStdString(key)).toInt();
    settings.endGroup();
}

void Config::write_value(std::string key, int value) {
    QSettings settings(SETTING_FILE, QSettings::IniFormat);
    settings.beginGroup(PROTO_GROUP);
    settings.setValue(QString::fromStdString(key), QString::number(value));
    settings.endGroup();
}
