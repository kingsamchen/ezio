
#include "catch2/catch.hpp"

#include "ezio/buffer.h"

#include <algorithm>
#include <cstring>

#include "kbase/string_view.h"

namespace {

constexpr size_t kSupposedPrepend = 8;
constexpr size_t kSupposedInitialSize = 1024;

}   // namespace

namespace ezio {

TEST_CASE("Construct a buffer", "[Buffer]")
{
    SECTION("use default sizes") {
        Buffer buf;
        REQUIRE(kSupposedPrepend == buf.prependable_size());
        REQUIRE(0 == buf.readable_size());
        REQUIRE(kSupposedInitialSize == buf.writable_size());
    }

    SECTION("use empty initial writable size") {
        Buffer buf(0);
        REQUIRE(kSupposedPrepend == buf.prependable_size());
        REQUIRE(0 == buf.readable_size());
        REQUIRE(0 == buf.writable_size());
    }

    SECTION("use specified writable size") {
        Buffer buf(32);
        REQUIRE(kSupposedPrepend == buf.prependable_size());
        REQUIRE(32 == buf.writable_size());
    }
}

TEST_CASE("Write data bytes to buffer", "[Buffer]")
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

TEST_CASE("Peek and consume data bytes from buffer", "[Buffer]")
{
    Buffer buf;
    REQUIRE(buf.readable_size() == 0);

    size_t written_bytes = 0;

    uint32_t magic = 0xDEADBEEF;
    buf.Write(&magic, sizeof(magic));
    written_bytes += sizeof(magic);

    REQUIRE(buf.readable_size() == written_bytes);

    std::string str("hello world");
    size_t str_len = str.length();
    buf.Write(&str_len, sizeof(str_len));
    buf.Write(str.data(), str_len);
    written_bytes += (sizeof(str_len) + str_len);

    REQUIRE(buf.readable_size() == written_bytes);

    // Let's peek and consume those written data bytes.

    CHECK(magic == *reinterpret_cast<const uint32_t*>(buf.Peek()));
    buf.Consume(sizeof(magic));
    written_bytes -= sizeof(magic);
    REQUIRE(buf.readable_size() == written_bytes);

    SECTION("consume data will enlarge prependable size") {
        REQUIRE(buf.prependable_size() > kSupposedPrepend);
    }

    auto read_str_len = *reinterpret_cast<const size_t*>(buf.Peek());
    CHECK(str_len == read_str_len);
    buf.Consume(sizeof(read_str_len));
    written_bytes -= sizeof(read_str_len);
    REQUIRE(buf.readable_size() == written_bytes);

    kbase::StringView sv(buf.Peek(), read_str_len);
    CHECK(sv == str);
    buf.Consume(read_str_len);
    written_bytes -= read_str_len;
    REQUIRE(written_bytes == 0);
    REQUIRE(buf.readable_size() == 0);

    SECTION("prependable size would be reset to the default once all data bytes are consumed") {
        REQUIRE(buf.prependable_size() == kSupposedPrepend);
    }
}

TEST_CASE("Read trivial integral and/or floating point values", "[Buffer]")
{
    Buffer buf;

    SECTION("read trivial integrals") {
        buf.Write(uint8_t(0xFF));
        buf.Write('a');
        buf.Write(short(-1));
        buf.Write(uint32_t(0xDEADBEEF));
        buf.Write(size_t(1024));

        REQUIRE(buf.ReadAs<uint8_t>() == uint8_t(0xFF));
        REQUIRE(buf.ReadAs<char>() == 'a');
        REQUIRE(buf.ReadAs<short>() == -1);
        REQUIRE(buf.ReadAs<uint32_t>() == 0xDEADBEEF);
        REQUIRE(buf.ReadAs<size_t>() == 1024);
    }

    SECTION("read floating points") {
        // exact hexadecimal representation is: 0x407e0000
        float f = 3.96875F;
        buf.Write(f);
        double d = 3.1415926;
        buf.Write(d);
        auto f_ep = buf.ReadAs<float>() - 3.96875F;
        REQUIRE(f_ep == 0);
        auto d_ep = std::abs(buf.ReadAs<double>() - d);
        REQUIRE(d_ep == 0);
    }
}

TEST_CASE("Read string or character sequence from buffer", "[Buffer]")
{
    Buffer buf;
    std::string s("hello world");
    buf.Write(s.data(), s.size());

    SECTION("read in given size") {
        auto s1 = buf.ReadAsString(5);
        auto s2 = buf.ReadAsString(1);
        auto s3 = buf.ReadAsString(5);
        CHECK(buf.readable_size() == 0);
        CHECK((s1 + s2 + s3) == s);
    }

    SECTION("read all as string") {
        auto str = buf.ReadAllAsString();
        CHECK(buf.readable_size() == 0);
        CHECK(s == str);
    }
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
    auto data_size = buf.ReadAs<int32_t>();
    auto str = buf.ReadAsString(static_cast<size_t>(data_size));
    REQUIRE(0 == buf.readable_size());
    REQUIRE(data_size == str.size());
    REQUIRE(s1 + s2 == str);
}

TEST_CASE("Iteratos", "[Buffer]")
{
    Buffer buf;

    REQUIRE(0 == buf.readable_size());
    REQUIRE(buf.begin() == buf.end());

    std::string s("hello world");
    buf.Write(s.data(), s.length());
    REQUIRE_FALSE(buf.begin() == buf.end());
    REQUIRE(static_cast<size_t>(std::distance(buf.begin(), buf.end())) == buf.readable_size());

    auto space_it = std::find(buf.begin(), buf.end(), ' ');
    REQUIRE(space_it != buf.end());
    REQUIRE(' ' == *space_it);
    REQUIRE('w' == *std::next(space_it));
    REQUIRE(std::string(buf.begin(), space_it) == "hello");
}

}   // namespace ezio
