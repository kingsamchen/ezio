/*
 @ 0xCCCCCCCC
*/

#ifndef DATA_CODEC_H_
#define DATA_CODEC_H_

#include <functional>

#include "kbase/basic_macros.h"
#include "kbase/string_view.h"

#include "ezio/chrono_utils.h"
#include "ezio/tcp_connection.h"

constexpr kbase::StringView kCmdList {"LIST"};
constexpr kbase::StringView kCmdName {"NAME"};
constexpr kbase::StringView kCmdUseName {"USE-NAME"};

class DataCodec {
public:
    enum DataType : int8_t {
        TypeBegin,
        Command,
        Message,
        TypeEnd
    };

    using CommandHandler = std::function<void(const ezio::TCPConnectionPtr&, const std::string&,
                                              ezio::TimePoint)>;
    using MessageHandler = std::function<void(const ezio::TCPConnectionPtr&, const std::string&,
                                              ezio::TimePoint)>;

    DataCodec() = default;

    ~DataCodec() = default;

    DISALLOW_COPY(DataCodec);

    DISALLOW_MOVE(DataCodec);

    static bool MatchCommand(const std::string& str, std::string& cmd);

    static std::string NewMessage(const std::string& msg);

    static std::string NewCommand(const std::string& command);

    void set_on_command(CommandHandler handler)
    {
        on_command_ = std::move(handler);
    }

    void set_on_message(MessageHandler handler)
    {
        on_message_ = std::move(handler);
    }

    void OnDataReceive(const ezio::TCPConnectionPtr& conn, ezio::Buffer& buf,
                       ezio::TimePoint ts) const;

private:
    void DispatchParsedData(DataType type,
                            const std::string& data,
                            const ezio::TCPConnectionPtr& conn,
                            ezio::TimePoint ts) const;

private:
    static constexpr size_t kDataLenTypeSize = sizeof(uint16_t);

    CommandHandler on_command_;
    MessageHandler on_message_;
};

#endif  // DATA_CODEC_H_
