#include "pb_tool.h"
#include <set>
#include <json/json.h>
#include <functional>
#include <sstream>
#include <iostream>
#include <stdint.h>
#include <QByteArray>
#include <QRegExp>
#include <google/protobuf/util/json_util.h>

PbTool::PbTool()
{

}

PbTool::~PbTool()
{

}

std::string PbTool::pb_load_proto(const std::string& filename, std::vector<std::string>& out)
{
    //TODO 检查文件名是否合法
    auto pos = filename.find_last_of('/');
    std::string path;
    std::string file;
    if(pos == std::string::npos) {
        file = filename;
    } else {
        path = filename.substr(0, pos);
        file = filename.substr(pos + 1);
    }

    if("" == path || "" == file) {
        return "[ERROR] proto文件路径错误!";
    }

    ::google::protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", path);
    ::google::protobuf::compiler::Importer importer(&sourceTree, NULL);
    const ::google::protobuf::FileDescriptor* file_descriptor = importer.Import(file);
    if(!file_descriptor) {
        return "[ERROR] 加载proto文件失败!";
    }

    if(file_descriptor->package() == "") {
        return "[ERROR] 加载proto文件失败! 请确保已经指定了package包名。\n格式如下: \nsyntax = \"proto3\"; \n\npackage demo; \n\n....";
    }

    for (int i = 0; i < file_descriptor->message_type_count(); ++i) {
        out.push_back(file_descriptor->message_type(i)->full_name());

        // 从proto文件中读取所有 bytes 字段名集合
        const google::protobuf::Descriptor* descriptor = file_descriptor->message_type(i);
        for(int i = 0; i < descriptor->field_count(); ++i) {
            const google::protobuf::FieldDescriptor* field = descriptor->field(i);
            if (field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES) {
                m_bytes_fields.insert(field->name());
            }
        }
    }

    return "[INFO] Load proto file success.";
}

// 调用google官方函数，将 json 转 message
bool PbTool::json_to_proto_message(const std::string& json, ::google::protobuf::Message* msg) {
    ::google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    options.always_print_enums_as_ints = true;
    return google::protobuf::util::JsonStringToMessage(json, msg).ok();
}

// 调用google官方函数，将 message 转 json
bool PbTool::proto_message_to_json(const ::google::protobuf::Message* msg, std::string& json, bool all_print) {
    ::google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = all_print; // 是否打印所有字段
    options.preserve_proto_field_names = true;
    options.always_print_enums_as_ints = true;
    return ::google::protobuf::util::MessageToJsonString(*msg, &json, options).ok();
}

/**
 * 自定义json字符串编码
 * protobuf内部自动将 bytes 数据先进行Base64编码，再放到json里
 * 这里做特殊处理，将十六进制字符串先转成bytes，再用Base64编码后放到json里
 */
void PbTool::custom_encode_json(Json::Value& jnode) {
    if (jnode.size() > 0) {
        std::string hex;
        std::string bs;

        for (Json::Value::iterator iter = jnode.begin(); iter != jnode.end(); iter++) {
            if (iter->isString()) {
                hex = iter->asString();

                if (IsHexString(hex) && IsBytesField(iter.key().asString())) {
                    bs = QByteArray::fromHex(hex.c_str()).toBase64().toStdString();
                    iter->copy(bs); // Hex转Base64字符串
                }

            } else if (iter->isArray()) {
                custom_encode_json(*iter);

            } else if (iter->isObject()) {
                custom_encode_json(*iter);
            }
        }
    }
}

/**
 * 自定义json字符串解码
 * protobuf内部自动将 bytes 数据先进行Base64编码，再放到json里
 * 这里做特殊处理，将Base64解码后转成十六进制字符串，再放到json里
 */
void PbTool::custom_decode_json(Json::Value& jnode) {
    if (jnode.size() > 0) {
        std::string bs;
        std::string hex;
        std::string key;

        for (Json::Value::iterator iter = jnode.begin(); iter != jnode.end(); iter++) {
            if (iter->isString()) {
                bs = iter->asString();

                if (IsBase64String(bs) && IsBytesField(iter.key().asString())) {
                    hex = QByteArray::fromBase64(bs.c_str()).toHex().toUpper().toStdString();
                    iter->copy(hex); // Base64转Hex字符串
                }

            } else if (iter->isArray()) {
                custom_decode_json(*iter);

            } else if (iter->isObject()) {
                custom_decode_json(*iter);
            }
        }
    }
}

std::string PbTool::pb_encode(const std::string& filename, const std::string& classname, const std::string input, std::string& output) {

    std::string result = DynamicParseFromPBFile(filename, classname,
        [this, input, &output](::google::protobuf::Message* msg) {
        std::string str;

        Json::Value jnode;
        if (StringToJson(input, jnode)) {
            this->custom_encode_json(jnode);

            // 删除cmd转的十六进制字符串
            //if (jnode.isMember("cmd (hex)")) {
            //    jnode.removeMember("cmd (hex)");
            //}
            str = JsonToString(jnode);
        }

        if (!this->json_to_proto_message(str, msg)) {
            return "[ERROR] json 转 proto 数据失败，请检查格式是否正确！";
        }
        if (msg->SerializeToString(&str)) {
            output = this->ByteToHexStr((unsigned char*)str.c_str(), str.length());
        } else {
            return "[ERROR] input format string failed to encode!";
        }

        return "[INFO] 编码成功.";
    });

    return result;
}

std::string PbTool::pb_decode(const std::string& filename, const std::string& classname, const std::string input, std::string& output, bool all_print) {

    int length = input.length();
    unsigned char *pb_data = new unsigned char[length];
    HexStrToByte(input.c_str(), pb_data, length);

    std::string result = DynamicParseFromPBFile(filename, classname,
        [this, pb_data, length, &output, all_print](::google::protobuf::Message* msg) {
        std::string str;
        if (msg->ParseFromArray(pb_data, length/2)) {
            if (!this->proto_message_to_json(msg, str, all_print)) {
                return "[ERROR] proto 转 json 数据失败，请检查数据是否正确！";
            }

            Json::Value jnode;
            if (StringToJson(str, jnode)) {
                this->custom_decode_json(jnode);

                // 插入cmd转十六进制字符串
                //if (jnode.isMember("cmd")) {
                //    const Json::Value cmd = jnode["cmd"];
                //    if (cmd.isUInt() && cmd.asUInt() > 0) {
                //        jnode["cmd (hex)"] = this->DecstrToHexStr(cmd.asUInt());
                //    }
                //}
                str = JsonToString(jnode);
            }
        } else {
            return "[ERROR] input format string failed to decode!";
        }

        output = str;
        return "[INFO] 解码成功.";
    }, all_print);

    delete []pb_data;
    return result;
}

std::string PbTool::pb_decode_empty(const std::string& filename, const std::string& classname, std::string& output) {

    std::string result = DynamicParseFromPBFile(filename, classname,
        [this, &output](::google::protobuf::Message* msg) {
        // 测试空HEX: "0D00000000"
        unsigned char pb_data[5] = {0x0D, 0x00, 0x00, 0x00, 00};

        if (msg->ParseFromArray(pb_data, 5)) {
            if (!this->proto_message_to_json(msg, output, true)) {
                return "[ERROR] proto 转 json 格式字符串失败！";
            }
        } else {
            return "[ERROR] print json format string fail!";
        }
        return "";
    });

    return result;
}

std::string PbTool::DynamicParseFromPBFile(const std::string& filename
    , const std::string& classname
    , std::function<std::string(::google::protobuf::Message* msg)> cb, bool all_print) {
    //TODO 检查文件名是否合法
    auto pos = filename.find_last_of('/');
    std::string path;
    std::string file;
    if(pos == std::string::npos) {
        file = filename;
    } else {
        path = filename.substr(0, pos);
        file = filename.substr(pos + 1);
    }

    if("" == path || "" == file) {
        return "[ERROR] proto文件路径错误!";
    }
    //printf("path = %s, file = %s\n", path.c_str(), file.c_str());

    ::google::protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", path);
    ::google::protobuf::compiler::Importer importer(&sourceTree, NULL);
    importer.Import(file);
    const ::google::protobuf::Descriptor *descriptor
        = importer.pool()->FindMessageTypeByName(classname);
    if(!descriptor) {
        return "[ERROR] Load proto message class fail!";
    }
    ::google::protobuf::DynamicMessageFactory factory;
    const ::google::protobuf::Message *message
        = factory.GetPrototype(descriptor);
    if(!message) {
        return "[ERROR] Parse proto message fail!";
    }
    ::google::protobuf::Message* msg = message->New();
    if(!msg) {
        return "[ERROR] Construct a new instance of the message type fail!";
    }

    std::string result;
    if(cb) {
        result = cb(msg);
    }
    delete msg;
    return result;
}

std::string PbTool::JsonToString(const Json::Value& json) {
    Json::FastWriter w;
    return w.write(json);
}

bool PbTool::StringToJson(const std::string& str, Json::Value& json) {
    Json::Reader r;
    return r.parse(str, json);
}

void PbTool::HexStrToByte(const char* source, unsigned char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i += 2) {
        highByte = toupper(source[i]);
        lowByte = toupper(source[i + 1]);

        if (highByte > 0x39)
        highByte -= 0x37;
        else
        highByte -= 0x30;

        if (lowByte > 0x39)
        lowByte -= 0x37;
        else
        lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
}

std::string PbTool::ByteToHexStr(const unsigned char* data, const int datasize)
{
    std::string output;
    char ch[3];

    for(int i = 0; i < datasize; ++i) {
        sprintf_s(ch, 3, "%02X", data[i]);
        output += ch;
    }
    return output;
}

// 整数转十六进制字符串
std::string PbTool::DecstrToHexStr(uint32_t num)
{
    std::string ret = "0x";
    for(int i = 3; i >= 0; --i)
    {
        ret.push_back("0123456789ABCDEF"[(((uint8_t*)&num)[i] >> 4) & 0xF]);
        ret.push_back("0123456789ABCDEF"[((uint8_t*)&num)[i] & 0xF]);
    }
    return ret;
}

bool PbTool::IsBase64String(const std::string& str)
{
    int length = str.length();
    if (length == 0 || length % 4 != 0) {
        return false;
    }

    char ch;
    for(int i = 0; i < length; ++i) {
        ch = str[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')
            || ch == '+' || ch == '/' || ch == '=') {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool PbTool::IsHexString(const std::string& str)
{
    int length = str.length();
    if (length == 0 || length % 2 != 0) {
        return false;
    }

    char c;
    for(int i = 0; i < length; ++i) {
        c = str[i];
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9')) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool PbTool::IsBytesField(const std::string& field_name)
{
    if(m_bytes_fields.find(field_name) != m_bytes_fields.end()) {
        return true;
    }
    return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
///
//// serialize message to json
//void PbTool::serialize_message(const google::protobuf::Message& message, Json::Value& jnode) {
//    const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
//    const google::protobuf::Reflection* reflection = message.GetReflection();

//    for(int i = 0; i < descriptor->field_count(); ++i) {
//        const google::protobuf::FieldDescriptor* field = descriptor->field(i);

//        if(field->is_repeated()) {
//            if(!reflection->FieldSize(message, field)) {
//                continue;
//            }
//        } else {
//            if(!reflection->HasField(message, field)) {
//                continue;
//            }
//        }

//        if(field->is_repeated()) {
//            switch(field->cpp_type()) {
//#define XX(cpptype, method, valuetype, jsontype) \
//                case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: { \
//                    int size = reflection->FieldSize(message, field); \
//                    for(int n = 0; n < size; ++n) { \
//                        jnode[field->name()].append((jsontype)reflection->GetRepeated##method(message, field, n)); \
//                    } \
//                    break; \
//                }
//            XX(INT32, Int32, int32_t, Json::Int);
//            XX(UINT32, UInt32, uint32_t, Json::UInt);
//            XX(FLOAT, Float, float, double);
//            XX(DOUBLE, Double, double, double);
//            XX(BOOL, Bool, bool, bool);
//            XX(INT64, Int64, int64_t, Json::Int64);
//            XX(UINT64, UInt64, uint64_t, Json::UInt64);
//#undef XX
//                case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
//                    int size = reflection->FieldSize(message, field);
//                    for(int n = 0; n < size; ++n) {
//                        jnode[field->name()].append(reflection->GetRepeatedEnum(message, field, n)->number());
//                    }
//                    break;
//                }
//                case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
//                    int size = reflection->FieldSize(message, field);
//                    for(int n = 0; n < size; ++n) {
//                        jnode[field->name()].append(reflection->GetRepeatedString(message, field, n));
//                    }
//                    break;
//                }
//                case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
//                    int size = reflection->FieldSize(message, field);
//                    for(int n = 0; n < size; ++n) {
//                        Json::Value vv;
//                        serialize_message(reflection->GetRepeatedMessage(message, field, n), vv);
//                        jnode[field->name()].append(vv);
//                    }
//                    break;
//                }
//            }
//            continue;
//        }

//        switch(field->cpp_type()) {
//#define XX(cpptype, method, valuetype, jsontype) \
//            case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: { \
//                jnode[field->name()] = (jsontype)reflection->Get##method(message, field); \
//                break; \
//            }
//            XX(INT32, Int32, int32_t, Json::Int);
//            XX(UINT32, UInt32, uint32_t, Json::UInt);
//            XX(FLOAT, Float, float, double);
//            XX(DOUBLE, Double, double, double);
//            XX(BOOL, Bool, bool, bool);
//            XX(INT64, Int64, int64_t, Json::Int64);
//            XX(UINT64, UInt64, uint64_t, Json::UInt64);
//#undef XX
//            case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
//                jnode[field->name()] = reflection->GetEnum(message, field)->number();
//                break;
//            }
//            case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
//                if(field->type() == google::protobuf::FieldDescriptor::TYPE_BYTES) {
//                    std::string scratch;
//                    const std::string data = reflection->GetStringReference(message, field, &scratch);
//                    jnode[field->name()] = ByteToHexStr((unsigned char*)data.c_str(), data.size());
//                } else {
//                    jnode[field->name()] = reflection->GetString(message, field);
//                }
//                break;
//            }
//            case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
//                serialize_message(reflection->GetMessage(message, field), jnode[field->name()]);
//                break;
//            }
//        }

//    }

//    const auto& ufs = reflection->GetUnknownFields(message);
//    serialize_unknowfieldset(ufs, jnode);
//}

//void PbTool::serialize_unknowfieldset(const google::protobuf::UnknownFieldSet& ufs, Json::Value& jnode) {
//    std::map<int, std::vector<Json::Value> > kvs;
//    for(int i = 0; i < ufs.field_count(); ++i) {
//        const auto& uf = ufs.field(i);
//        switch(uf.type()) {
//            case google::protobuf::UnknownField::TYPE_VARINT:
//                kvs[uf.number()].push_back((Json::Int64)uf.varint());
//                //jnode[std::to_string(uf.number())] = (Json::Int64)uf.varint();
//                break;
//            case google::protobuf::UnknownField::TYPE_FIXED32:
//                kvs[uf.number()].push_back((Json::UInt)uf.fixed32());
//                //jnode[std::to_string(uf.number())] = (Json::Int)uf.fixed32();
//                break;
//            case google::protobuf::UnknownField::TYPE_FIXED64:
//                kvs[uf.number()].push_back((Json::UInt64)uf.fixed64());
//                //jnode[std::to_string(uf.number())] = (Json::Int64)uf.fixed64();
//                break;
//            case google::protobuf::UnknownField::TYPE_LENGTH_DELIMITED:
//                google::protobuf::UnknownFieldSet tmp;
//                auto& v = uf.length_delimited();
//                if(!v.empty() && tmp.ParseFromString(v)) {
//                    Json::Value vv;
//                    serialize_unknowfieldset(tmp, vv);
//                    kvs[uf.number()].push_back(vv);
//                    //jnode[std::to_string(uf.number())] = vv;
//                } else {
//                    //jnode[std::to_string(uf.number())] = v;
//                    kvs[uf.number()].push_back(v);
//                }
//                break;
//        }
//    }

//    for(auto& i : kvs) {
//        if(i.second.size() > 1) {
//            for(auto& n : i.second) {
//                jnode[std::to_string(i.first)].append(n);
//            }
//        } else {
//            jnode[std::to_string(i.first)] = i.second[0];
//        }
//    }
//}

//// 从message中读取所有 bytes 字段名集合
//void PbTool::GetMessageBytesFields(const google::protobuf::Message& message, std::set<std::string>& bytesFieldSet) {
//    const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
//    const google::protobuf::Reflection* reflection = message.GetReflection();

//    for(int i = 0; i < descriptor->field_count(); ++i) {
//        const google::protobuf::FieldDescriptor* field = descriptor->field(i);

//        int type = field->type();
//        if (type == google::protobuf::FieldDescriptor::TYPE_BYTES) {
//            bytesFieldSet.insert(field->name());
//        } else if (type == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
//            if (field->is_repeated()) {
//                if(!reflection->FieldSize(message, field)) {
//                    continue;
//                }
//                const google::protobuf::Message& msg = reflection->GetRepeatedMessage(message, field, i);
//                GetMessageBytesFields(msg, bytesFieldSet);
//            } else {
//                const google::protobuf::Message& msg = reflection->GetMessage(message, field);
//                GetMessageBytesFields(msg, bytesFieldSet);
//            }
//        }
//    }
//}

/////////////////////////////////////////////////////////////////////////////////////////////////
