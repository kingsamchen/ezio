/*
 @ 0xCCCCCCCC
*/

#include "ezio/buffer.h"

namespace ezio {

Buffer::Buffer()
    : Buffer(kDefaultInitialSize)
{}

Buffer::Buffer(size_t initial_size)
    : buf_(initial_size + kDefaultPrependSize),
      reader_index_(kDefaultPrependSize),
      writer_index_(kDefaultPrependSize)
{}

void Buffer::Write(const void* data, size_t size)
{
    ReserveWritable(size);
    ENSURE(CHECK, writable_size() >= size)(writable_size())(size).Require();

    memcpy(BeginWrite(), data, size);
    EndWrite(size);
}

void Buffer::Consume(size_t data_size)
{
    ENSURE(CHECK, data_size <= readable_size())(data_size)(readable_size()).Require();
    if (data_size < readable_size()) {
        reader_index_ += data_size;
    } else {
        ConsumeAll();
    }
}

std::string Buffer::ReadAsString(size_t length)
{
    ENSURE(CHECK, readable_size() >= length)(readable_size())(length).Require();
    auto b = begin();
    std::string s(b, b + length);
    Consume(length);
    return s;
}

std::string Buffer::ReadAllAsString()
{
    std::string s(begin(), end());
    ConsumeAll();
    return s;
}

void Buffer::Prepend(const void* data, size_t size)
{
    ENSURE(CHECK, prependable_size() >= size)(prependable_size())(size).Require();
    auto start = reader_index_ - size;
    memcpy(buf_.data() + start, data, size);
    reader_index_ -= size;
}

void Buffer::Prepend(int16_t n)
{
    auto be = HostToNetwork(n);
    Prepend(&be, sizeof(be));
}

void Buffer::Prepend(int32_t n)
{
    auto be = HostToNetwork(n);
    Prepend(&be, sizeof(be));
}

void Buffer::Prepend(int64_t n)
{
    auto be = HostToNetwork(n);
    Prepend(&be, sizeof(be));
}

void Buffer::ReserveWritable(size_t new_size)
{
    if (writable_size() >= new_size) {
        return;
    }

    if (prependable_size() + writable_size() < kDefaultPrependSize + new_size) {
        buf_.resize(writer_index_ + new_size);
    } else {
        // Ranges may overlap.
        auto data_size = readable_size();
        memmove(buf_.data() + kDefaultPrependSize, buf_.data() + reader_index_, data_size);
        reader_index_ = kDefaultPrependSize;
        writer_index_ = reader_index_ + data_size;
    }
}

char* Buffer::BeginWrite()
{
    return buf_.data() + writer_index_;
}

void Buffer::EndWrite(size_t written_size)
{
    ENSURE(CHECK, writable_size() >= written_size).Require();
    writer_index_ += written_size;
}

}   // namespace ezio
