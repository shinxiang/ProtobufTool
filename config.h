#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config
{
public:
    Config();
    Config(int jsonPrintAll, int headLength, int tailLength);
    ~Config();

    void load_config(); // 加载配置
    void save_config(); // 保存配置

    void read_value(std::string key, std::string& value); // 读配置文件
    void write_value(std::string key, std::string value); // 写配置文件

    void read_value(std::string key, int& value);
    void write_value(std::string key, int value);

public:
    std::string m_protoFileName;  // 加载的proto文件名

    int m_jsonPrintAll;  // JSON字段：是否全部显示（1是，0否）
    int m_headLength;    // 报文格式：帧头长度（字节）
    int m_tailLength;    // 报文格式：帧尾长度（字节）
};

#endif // CONFIG_H
