/*
 @ 0xCCCCCCCC
*/

#include "data_codec.h"

#include <regex>

#include "kbase/error_exception_util.h"
#include "kbase/logging.h"

#include "ezio/buffer.h"

void DataCodec::OnDataReceive(const ezio::TCPConnectionPtr& conn, ezio::Buffer& buf,
                              ezio::TimePoint ts) const
{
    while (buf.readable_size() >= (sizeof(DataType) + kDataLenTypeSize)) {
        auto data_type = static_cast<DataType>(buf.PeekAs<int8_t>());
        if (!(DataType::TypeBegin < data_type && data_type < DataType::TypeEnd)) {
            LOG(ERROR) << "Invalid data type: " << data_type;
            conn->Shutdown();
            return;
        }

        auto data_len = ezio::NetworkToHost(
            *reinterpret_cast<const int16_t*>(buf.Peek() + sizeof(DataType)));
        if (data_len <= 0) {
            LOG(ERROR) << "Invalid data length: " << data_len;
            conn->Shutdown();
            return;
        }

        if (buf.readable_size() >= (sizeof(DataType) + kDataLenTypeSize + data_len)) {
            buf.Consume(sizeof(DataType));
            buf.Consume(kDataLenTypeSize);
            auto data = buf.ReadAsString(data_len);
            DispatchParsedData(data_type, data, conn, ts);
        }
    }
}

void DataCodec::DispatchParsedData(DataType type, const std::string& data,
                                   const ezio::TCPConnectionPtr& conn, ezio::TimePoint ts) const
{
    switch (type) {
        case DataType::Command:
            on_command_(conn, data, ts);
            break;

        case DataType::Message:
            on_message_(conn, data, ts);
            break;

        default:
            ENSURE(CHECK, kbase::NotReached()).Require();
            break;
    }
}

// static
bool DataCodec::MatchCommand(const std::string& str, std::string& cmd)
{
    std::smatch result;
    if (!std::regex_match(str, result, std::regex(R"(^\s*\$([\w\-]+)\$(\s+([\w\d\-_]+))?\s*$)"))) {
        return false;
    }

    cmd = result[1].str();
    if (result[3].matched) {
        ENSURE(CHECK, result.size() == 4)(result.size()).Require();
        cmd.append(" ").append(result[3]);
    }

    return true;
}

// static
std::string DataCodec::NewMessage(const std::string& msg)
{
    ezio::Buffer buf(64);
    buf.Write(static_cast<int8_t>(DataType::Message));
    buf.Write(static_cast<int16_t>(msg.size()));
    buf.Write(msg.data(), msg.size());

    return buf.ReadAllAsString();
}

// static
std::string DataCodec::NewCommand(const std::string& command)
{
    ezio::Buffer buf(16);
    buf.Write(static_cast<int8_t>(DataType::Command));
    buf.Write(static_cast<int16_t>(command.size()));
    buf.Write(command.data(), command.size());

    return buf.ReadAllAsString();
}
