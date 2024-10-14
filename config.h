#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config
{
public:
    Config();
    ~Config();

    void read_config(std::string key, std::string& value); // 读配置文件
    void write_config(std::string key, std::string value); // 写配置文件
};

#endif // CONFIG_H
