#ifndef PB_TOOL_H
#define PB_TOOL_H

#include "json/value.h"
#include <string>
#include <vector>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

class PbTool
{
public:
    PbTool();
    ~PbTool();

public:
    std::string pb_load_proto(const std::string& filename, std::vector<std::string>& out);
    std::string pb_encode(const std::string& filename, const std::string& classname, const std::string input, std::string& output);
    std::string pb_decode(const std::string& filename, const std::string& classname, const std::string input, std::string& output, bool all_print=false);
    std::string pb_decode_empty(const std::string& filename, const std::string& classname, std::string& output);

    void custom_encode_json(Json::Value& jnode);
    void custom_decode_json(Json::Value& jnode);

protected:
    bool json_to_proto_message(const std::string& json, ::google::protobuf::Message* msg);
    bool proto_message_to_json(const ::google::protobuf::Message* msg, std::string& json, bool all_print=false);

    std::string DynamicParseFromPBFile(const std::string& filename
        , const std::string& classname
        , std::function<std::string(::google::protobuf::Message* msg)> cb
        , bool all_print=false);

    std::string JsonToString(const Json::Value& json);
    bool StringToJson(const std::string& str, Json::Value& json);
    void HexStrToByte(const char* source, unsigned char* dest, int sourceLen);
    std::string ByteToHexStr(const unsigned char* data, const int datasize);
    std::string DecstrToHexStr(uint32_t num);

    bool IsBase64String(const std::string& str);
    bool IsHexString(const std::string& str);
    bool IsBytesField(const std::string& field_name);

private:
    std::set<std::string> m_bytes_fields; // 所有bytes类型字段名集合

    // serialize message to json
    //void serialize_message(const google::protobuf::Message& message, Json::Value& jnode);
    //void serialize_unknowfieldset(const google::protobuf::UnknownFieldSet& ufs, Json::Value& jnode);

    //void GetMessageBytesFields(const google::protobuf::Message& message, std::set<std::string>& bytesFieldSet);
};

#endif // PB_TOOL_H
