
#include "catch2/catch.hpp"

#include "ezio/buffer.h"

#include <algorithm>
#include <cstring>

namespace {

constexpr size_t kSupposedPrepend = 8;
constexpr size_t kSupposedInitialSize = 1024;

}   // namespace

namespace ezio {

TEST_CASE("Constructors", "[Buffer]")
{
    Buffer buf;
    REQUIRE(kSupposedPrepend == buf.prependable_size());
    REQUIRE(0 == buf.readable_size());
    REQUIRE(kSupposedInitialSize == buf.writable_size());

    Buffer empty_buf(0);
    REQUIRE(kSupposedPrepend == empty_buf.prependable_size());
    REQUIRE(0 == empty_buf.readable_size());
    REQUIRE(0 == empty_buf.writable_size());

    Buffer another_buf(32);
    REQUIRE(kSupposedPrepend == another_buf.prependable_size());
    REQUIRE(32 == another_buf.writable_size());
}

TEST_CASE("General writings to buffer", "[Buffer]")
{
    // 8 prependable size + 16 initial writable size.
    Buffer buf(16);
    REQUIRE(8 == buf.prependable_size());
    REQUIRE(16 == buf.writable_size());

    buf.Write("a", 1);
    REQUIRE(1 == buf.readable_size());
    REQUIRE(15 == buf.writable_size());

    unsigned short s = 0xFFFF;
    buf.Write(&s, sizeof(s));
    REQUIRE(3 == buf.readable_size());
    REQUIRE(13 == buf.writable_size());

    unsigned int i = 0xDEADBEEF;
    buf.Write(&i, sizeof(i));
    REQUIRE(7 == buf.readable_size());
    REQUIRE(9 == buf.writable_size());

    buf.Write("ABCD", 4);
    REQUIRE(11 == buf.readable_size());
    REQUIRE(5 == buf.writable_size());
    REQUIRE(kSupposedPrepend == buf.prependable_size());
}

TEST_CASE("Iteratos", "[Buffer]")
{
    Buffer buf;

    REQUIRE(0 == buf.readable_size());
    REQUIRE(buf.begin() == buf.end());

    std::string s("hello world");
    buf.Write(s.data(), s.length());
    REQUIRE_FALSE(buf.begin() == buf.end());
    REQUIRE(std::distance(buf.begin(), buf.end()) == buf.readable_size());

    auto space_it = std::find(buf.begin(), buf.end(), ' ');
    REQUIRE(space_it != buf.end());
    REQUIRE(' ' == *space_it);
    REQUIRE('w' == *std::next(space_it));
    REQUIRE(std::string(buf.begin(), space_it) == "hello");
}

TEST_CASE("Consuming buffer", "[Buffer]")
{
    Buffer buf;

    unsigned int magic = 0xDEADBEEF;
    buf.Write(&magic, sizeof(magic));

    std::string s("hello world");
    size_t len = s.length();
    buf.Write(&len, sizeof(len));
    buf.Write(s.data(), len);

    size_t written_size = sizeof(magic) + sizeof(len) + len;
    REQUIRE(written_size == buf.readable_size());

    auto uptr = reinterpret_cast<const unsigned int*>(&(*buf.begin()));
    REQUIRE(0xDEADBEEF == *uptr);
    buf.Consume(sizeof(magic));

    auto len_ptr = reinterpret_cast<const size_t*>(&(*buf.begin()));
    REQUIRE(len == *len_ptr);
    buf.Consume(sizeof(len));

    REQUIRE(kSupposedPrepend + written_size - len == buf.prependable_size());

    REQUIRE(s == std::string(buf.begin(), std::next(buf.begin(), len)));
    buf.Consume(len);
    REQUIRE(0 == buf.readable_size());
    REQUIRE(kSupposedPrepend == buf.prependable_size());
}

TEST_CASE("WritesAndPeeks", "[Buffer]")
{
    Buffer buf;
    buf.Write(int8_t(0xFF));
    buf.Write(int16_t(-123));
    buf.Write(int32_t(0xDEADBEEF));
    buf.Write(int64_t(-1));
    auto written_size = sizeof(int8_t) + sizeof(int16_t) + sizeof(int32_t) + sizeof(int64_t);
    REQUIRE(written_size == buf.readable_size());

    REQUIRE(0xFF == static_cast<uint8_t>(buf.PeekAsInt8()));
    buf.Consume(sizeof(int8_t));

    REQUIRE(-123 == buf.PeekAsInt16());
    buf.Consume(sizeof(int16_t));

    REQUIRE(0xDEADBEEF == static_cast<unsigned>(buf.PeekAsInt32()));
    buf.Consume(sizeof(int32_t));

    REQUIRE(-1 == buf.PeekAsInt64());
}

TEST_CASE("Read from buffer", "[Buffer]")
{
    Buffer buf;
    std::string s("hello world");
    buf.Write(static_cast<int32_t>(s.length()));
    buf.Write(s.data(), s.size());

    auto length = static_cast<size_t>(buf.ReadAsInt32());
    auto str = buf.ReadAllAsString();
    REQUIRE(length == str.length());
    REQUIRE(s == str);
}

TEST_CASE("ReserveWritable", "[Buffer]")
{
    Buffer buf(16);
    std::string s("hello world");
    buf.Write(s.data(), s.size());
    auto it = std::find(buf.begin(), buf.end(), ' ');
    auto partial = buf.ReadAsString(static_cast<size_t>(std::distance(buf.begin(), ++it)));
    REQUIRE("hello " == partial);
    REQUIRE(16 - s.size() == buf.writable_size());
    REQUIRE(kSupposedPrepend + partial.size() == buf.prependable_size());

    std::string tail(" is flat");
    REQUIRE(tail.size() > buf.writable_size());
    buf.Write(tail.data(), tail.size());
    REQUIRE(kSupposedPrepend == buf.prependable_size());
    REQUIRE(16 - strlen("world is flat") == buf.writable_size());

    // We have writable-size = 3 now
    std::string ss("; the world has changed");
    buf.Write(ss.data(), ss.size());
    REQUIRE(0 == buf.writable_size());
    auto final = buf.ReadAllAsString();
    REQUIRE("world is flat" + ss == final);
    REQUIRE(0 == buf.readable_size());
}

TEST_CASE("Prepending", "[Buffer]")
{
    Buffer buf;
    std::string s1("Bilibili gaga");
    buf.Write(s1.data(), s1.size());
    std::string s2("Deep Dark Fantasy");
    buf.Write(s2.data(), s2.size());
    buf.Prepend(static_cast<int32_t>(buf.readable_size()));

    REQUIRE(4 == buf.prependable_size());
    auto data_size = buf.ReadAsInt32();
    auto str = buf.ReadAsString(static_cast<size_t>(data_size));
    REQUIRE(0 == buf.readable_size());
    REQUIRE(data_size == str.size());
    REQUIRE(s1 + s2 == str);
}

}   // namespace ezio
