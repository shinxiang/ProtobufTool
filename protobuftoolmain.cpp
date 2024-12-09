#include "protobuftoolmain.h"
#include "ui_protobuftoolmain.h"
#include "settingdialog.h"
#include <QJsonDocument>
#include <QFileDialog>
#include <QCompleter>
#include <QMessageBox>
//#include <qnamespace.h>

ProtobufToolMain::ProtobufToolMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ProtobufToolMain)
{
    ui->setupUi(this);
    pb_tool = new PbTool();

    QFont font(QStringLiteral("微软雅黑"), 10);

    // 设置输入文本框字体大小
    ui->plainTextEdit_left->setFont(font);
    // 设置输出文本框字体大小
    ui->plainTextEdit_right->setFont(font);
    // 设置下拉文本框字体大小
    ui->comboBox_messageType->setFont(font);

    // 设置comboBox下拉框可搜索过滤
    ui->comboBox_messageType->setEditable(true);
    QCompleter *completer = new QCompleter(ui->comboBox_messageType->model(), this);
    completer->setCaseSensitivity(Qt::CaseInsensitive); // 设置过滤条件不区分大小写
    ui->comboBox_messageType->setCompleter(completer);


    // 加载配置文件
    m_config = new Config();
    m_config->load_config();
    load_proto_message_type();
}

ProtobufToolMain::~ProtobufToolMain()
{
    delete ui;
    delete pb_tool;
    delete m_config;
}

bool ProtobufToolMain::checkProtoFilePath()
{
    if (m_config->m_protoFileName == "") {
        ui->plainTextEdit_tips->setPlainText(QString("请先加载proto文件！"));
        return false;
    }
    if (m_protoClassName == "") {
        ui->plainTextEdit_tips->setPlainText(QString("请先选择消息类型！"));
        return false;
    }
    return true;
}

// 编码
void ProtobufToolMain::on_pushButton_encode_clicked()
{
    if (!checkProtoFilePath()) {
        return;
    }
    ui->plainTextEdit_tips->clear();

    QString json = ui->plainTextEdit_right->toPlainText();
    if (json.isEmpty()) {
        ui->plainTextEdit_tips->setPlainText("请输入需要编码的 JSON 字符串！");
        return;
    }

    std::string hex;
    std::string ret = pb_tool->pb_encode(m_config->m_protoFileName, m_protoClassName, json.toStdString(), hex);
    ui->plainTextEdit_tips->setPlainText(QString::fromStdString(ret));

    ui->plainTextEdit_left->setPlainText(QString::fromStdString(hex));
}

// 解码
void ProtobufToolMain::on_pushButton_decode_clicked()
{
    if (!checkProtoFilePath()) {
        return;
    }
    ui->plainTextEdit_tips->clear();

    QString hex = ui->plainTextEdit_left->toPlainText();
    if (hex.isEmpty()) {
        ui->plainTextEdit_tips->setPlainText("请输入需要解码的 HEX 字符串！");
        return;
    }
    hex.remove(QRegExp("\\s")); // 去除HEX字符串中所有空格

    // 报文处理：截断帧头帧尾
    int length = hex.length();
    int trimLength = (m_config->m_headLength + m_config->m_tailLength) * 2;
    if (trimLength >= length) {
        ui->plainTextEdit_tips->setPlainText("[配置错误] 帧头帧尾字节长度>=报文总长度！");
        return;
    }
    hex = hex.mid(m_config->m_headLength * 2, length - trimLength);

    std::string json;
    std::string ret = pb_tool->pb_decode(m_config->m_protoFileName, m_protoClassName, hex.toStdString(), json, m_config->m_jsonPrintAll);
    ui->plainTextEdit_tips->setPlainText(QString::fromStdString(ret));

    if (!json.empty()) {
        // json格式化
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(json.c_str(), json.size()));
        QString formatJsonString = doc.toJson(QJsonDocument::Indented);
        ui->plainTextEdit_right->setPlainText(formatJsonString);
    }
}

// 加载proto文件按钮
void ProtobufToolMain::on_pushButton_openFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,QStringLiteral("文件对话框！"), QDir::currentPath(), QStringLiteral("文件(*proto);;""文本文件(*txt)"));
    m_config->m_protoFileName = fileName.toStdString();
    m_config->save_config(); // 修改配置

    load_proto_message_type();
}

void ProtobufToolMain::load_proto_message_type()
{
    ui->comboBox_messageType->clear();

    if (m_config->m_protoFileName != "") {
        ui->label_filepath->setTextInteractionFlags(Qt::TextSelectableByMouse);
        ui->label_filepath->setText(QString::fromStdString(("文件路径：" + m_config->m_protoFileName)));

        std::vector<std::string> vec;
        std::string ret = pb_tool->pb_load_proto(m_config->m_protoFileName, vec);
        ui->plainTextEdit_tips->setPlainText(QString::fromStdString(ret));

        // 重新加载下拉框
        if(!vec.empty()) {
            QStringList strList;
            QString msgType;
            for(std::vector<std::string>::iterator iter = vec.begin(); iter != vec.end(); ++iter) {
                msgType = QString::fromStdString(*iter);
                strList = msgType.split(".");
                if (strList.length() > 1) {
                    ui->comboBox_messageType->addItem(strList[1], msgType);
                } else {
                    ui->comboBox_messageType->addItem(msgType, msgType);
                }
            }
            ui->comboBox_messageType->setCurrentIndex(0);
            m_protoClassName = ui->comboBox_messageType->itemData(0).toString().toStdString();
        } else {
            QMessageBox::information(NULL, "提示信息", QString::fromStdString(ret), QMessageBox::Ok);
        }
    }
}

void ProtobufToolMain::on_comboBox_messageType_currentIndexChanged(int index)
{
    m_protoClassName = ui->comboBox_messageType->itemData(index).toString().toStdString();

    printEmptyMessageJson();
}

// 打印空值json格式字符串
void ProtobufToolMain::printEmptyMessageJson()
{
    std::string json;
    std::string ret = pb_tool->pb_decode_empty(m_config->m_protoFileName, m_protoClassName, json);
    if (!json.empty()) {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(json.c_str(), json.size()));
        QString formatJsonString = doc.toJson(QJsonDocument::Indented);
        ui->plainTextEdit_right->setPlainText(formatJsonString);
    }
    if (!ret.empty()) {
        ui->plainTextEdit_tips->setPlainText(QString::fromStdString(ret));
    }
}

void ProtobufToolMain::on_pushButton_clearLeft_clicked()
{
    ui->plainTextEdit_left->clear();
}


void ProtobufToolMain::on_pushButton_clearRight_clicked()
{
    ui->plainTextEdit_right->clear();
}

void ProtobufToolMain::on_pushButton_setting_clicked()
{
    SettingDialog child(this);
    child.setWindowTitle("设置");
    child.initSetting(m_config);
    child.setWindowFlag(Qt::WindowContextHelpButtonHint,false);
    connect(&child, &SettingDialog::sendConfig, this, &ProtobufToolMain::receiveConfig);

    child.exec();
}

// 接收子窗口发送的配置信息
void ProtobufToolMain::receiveConfig(Config* cfg)
{
    if (cfg != NULL) {
        m_config->m_jsonPrintAll = cfg->m_jsonPrintAll;
        m_config->m_headLength = cfg->m_headLength;
        m_config->m_tailLength = cfg->m_tailLength;
        m_config->save_config(); // 保存配置

        delete cfg;
        cfg = NULL;
    }
}
