#ifndef PROTOBUFTOOLMAIN_H
#define PROTOBUFTOOLMAIN_H

#include <QMainWindow>
#include "pb_tool.h"
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProtobufToolMain; }
QT_END_NAMESPACE

class ProtobufToolMain : public QMainWindow
{
    Q_OBJECT //支持元对象系统 moc->支持信号与槽

public:
    ProtobufToolMain(QWidget *parent = nullptr);
    ~ProtobufToolMain();

private slots:
    void on_pushButton_encode_clicked(); // 编码
    void on_pushButton_decode_clicked(); // 解码

    void on_pushButton_openFile_clicked(); // 加载proto文件按钮
    void load_proto_message_type();      // 加载proto消息类型

    void on_comboBox_messageType_currentIndexChanged(int index);

    void on_pushButton_clearLeft_clicked();

    void on_pushButton_clearRight_clicked();


protected:
    bool checkProtoFilePath();
    void printEmptyMessageJson();

private:
    Ui::ProtobufToolMain *ui;
    PbTool *pb_tool;
    Config *m_config;


    std::string m_protoFileName;  // 加载的proto文件名
    std::string m_protoClassName; // 加载的proto类名
};
#endif // PROTOBUFTOOLMAIN_H
