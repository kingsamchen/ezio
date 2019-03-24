/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_BUFFER_H_
#define EZIO_BUFFER_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include "kbase/basic_macros.h"
#include "kbase/error_exception_util.h"

#include "ezio/endian_utils.h"

namespace ezio {

namespace internal {

struct single_byte {};
struct multi_byte {};

template<typename T>
struct floating_point_store_type;

template<>
struct floating_point_store_type<float> {
    using type = uint32_t;
};

template<>
struct floating_point_store_type<double> {
    using type = uint64_t;
};

}   // namespace internal

// For implementing application-layer buffering for tcp connections.
// Buffer provides support of random-access read-only iterators for access to readable
// content without modifying it inadvertently.
// Writing to the Buffer will internally convert numerical values into network endian
// byte order; and reading from the Buffer will convert them back into host endian byte
// order.
// Underlying structure layout:
//       <-------- initial size ----->
// +-----+------------+-------...----+
// |  p  |    ###     |              |
// +-----+------------+-------...----+
// 0     r            w              e
// prependable_size = r - 0
// readable_size = w - r
// writable_size = e - w

class Buffer {
private:
    static constexpr size_t kDefaultPrependSize = 8;
    static constexpr size_t kDefaultInitialSize = 1024;

public:
    using value_type = char;

    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Buffer::value_type;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;

        explicit Iterator(pointer ptr) noexcept
            : ptr_(ptr)
        {}

        ~Iterator() = default;

        reference operator*() const noexcept
        {
            return *ptr_;
        }

        pointer operator->() const noexcept
        {
            return ptr_;
        }

        reference operator[](difference_type n) const noexcept
        {
            return *(ptr_ + n);
        }

        Iterator& operator++() noexcept
        {
            ++ptr_;
            return *this;
        }

        Iterator& operator--() noexcept
        {
            --ptr_;
            return *this;
        }

        Iterator& operator+=(difference_type n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        Iterator& operator-=(difference_type n) noexcept
        {
            ptr_ -= n;
            return *this;
        }

        friend difference_type operator-(Iterator lhs, Iterator rhs) noexcept
        {
            return lhs.ptr_ - rhs.ptr_;
        }

        friend bool operator==(Iterator lhs, Iterator rhs) noexcept
        {
            return lhs.ptr_ == rhs.ptr_;
        }

        friend bool operator!=(Iterator lhs, Iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        pointer ptr_;
    };

    using const_iterator = Iterator;
    using iterator = const_iterator;

    Buffer();

    // `initial_size` doesn't include prependable size.
    explicit Buffer(size_t initial_size);

    ~Buffer() = default;

    DEFAULT_COPY(Buffer);

    DEFAULT_MOVE(Buffer);

    size_t prependable_size() const noexcept
    {
        return reader_index_;
    }

    size_t readable_size() const noexcept
    {
        return writer_index_ - reader_index_;
    }

    size_t writable_size() const noexcept
    {
        return buf_.size() - writer_index_;
    }

    iterator begin() const noexcept
    {
        return cbegin();
    }

    const_iterator cbegin() const noexcept
    {
        return iterator(buf_.data() + reader_index_);
    }

    iterator end() const noexcept
    {
        return cend();
    }

    const_iterator cend() const noexcept
    {
        return iterator(buf_.data() + writer_index_);
    }

    // Writes data bytes into the buffer.
    void Write(const void* data, size_t size);

    // Writes an integral value or a floating point into the buffer.
    // The function internally converts data bytes of the `value`, provided more than 1 byte,
    // into big-endian.
    // Floating points are 'interpreted' as corresponding size-equivalent unsigned integers
    // by using floating_point_store_type, and endianess is also handled as if it is a normal
    // integer. For portability, IEEE 754 spec is enforced for floating points.
    template<typename T>
    void Write(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "T must be integral or floating point");
        using byte_trait =
            std::conditional_t<sizeof(T) == 1, internal::single_byte, internal::multi_byte>;
        WriteImpl(value, byte_trait{});
    }

    // Returns pointer to the starting address of readable content.
    const value_type* Peek() const noexcept
    {
        return &*begin();
    }

    // Similar to ReadAs() but without consuming data bytes.
    template<typename T>
    T PeekAs() const
    {
        static_assert(std::is_arithmetic<T>::value, "T must be integral or floating point");
        using byte_trait =
            std::conditional_t<sizeof(T) == 1, internal::single_byte, internal::multi_byte>;
        return PeekAsImpl<T>(byte_trait{});
    }

    // If `data_size` is larger than or equal to readable_size(), then this function is
    // equivalent to ConsumeAll().
    void Consume(size_t data_size);

    // Consumes all readable data in buffer and reset prependable size to the default.
    void ConsumeAll() noexcept
    {
        reader_index_ = kDefaultPrependSize;
        writer_index_ = reader_index_;
    }

    // Reads an integral value or a floating point value out from the buffer.
    // The value, provided more than 1 bytes, will be converted back into little-endian.
    // Type restrictions applied for Write() function template are also applied here.
    template<typename T>
    T ReadAs()
    {
        static_assert(std::is_arithmetic<T>::value, "T must be integral or floating point");
        T n = PeekAs<T>();
        Consume(sizeof(n));
        return n;
    }

    // Reads data bytes in given length as a string.
    std::string ReadAsString(size_t length);

    std::string ReadAllAsString();

    void Prepend(const void* data, size_t size);

    void Prepend(int16_t n);

    void Prepend(int32_t n);

    void Prepend(int64_t n);

    // Returns pointer to the starting address of the writable space.
    // Use this function only when absolute necessary.
    char* BeginWrite();

    // If a write is done by copying data directly to the writable space returned by
    // BeginWrite(), then this function must be called to complete this data writing.
    void EndWrite(size_t written_size);

    // If writable size is not less than `new_size`, this function does nothing.
    // Otherwise, it will make sure there is enough writable space for `new_size` bytes.
    void ReserveWritable(size_t new_size);

private:
    template<typename T>
    void WriteImpl(T value, internal::single_byte)
    {
        static_assert(sizeof(T) == 1, "Require sizeof(T) == 1");
        Write(&value, sizeof(value));
    }

    template<typename I, std::enable_if_t<std::is_integral<I>::value, int> = 0>
    void WriteImpl(I value, internal::multi_byte)
    {
        static_assert(sizeof(I) > 1, "Require sizeof(I) > 1");
        auto be = HostToNetwork(value);
        Write(&be, sizeof(be));
    }

    template<typename F, std::enable_if_t<std::is_floating_point<F>::value, int> = 0>
    void WriteImpl(F value, internal::multi_byte)
    {
        static_assert(sizeof(F) > 1, "Require sizeof(F) > 1");
        static_assert(std::numeric_limits<F>::is_iec559, "IEEE 754 spec is enforced");
        using store_type = typename internal::floating_point_store_type<F>::type;
        WriteImpl(*reinterpret_cast<store_type*>(&value), internal::multi_byte{});
    }

    template<typename T>
    T PeekAsImpl(internal::single_byte) const
    {
        static_assert(sizeof(T) == 1, "Require sizeof(T) == 1");
        ENSURE(CHECK, readable_size() >= sizeof(T))(readable_size()).Require();
        T n = *reinterpret_cast<const T*>(Peek());
        return n;
    }

    template<typename I, std::enable_if_t<std::is_integral<I>::value, int> = 0>
    I PeekAsImpl(internal::multi_byte) const
    {
        static_assert(sizeof(I) > 1, "Require sizeof(I) > 1");
        ENSURE(CHECK, readable_size() >= sizeof(I))(readable_size())(sizeof(I)).Require();
        I be;
        memcpy(&be, Peek(), sizeof(be));
        return NetworkToHost(be);
    }

    template<typename F, std::enable_if_t<std::is_floating_point<F>::value, int> = 0>
    F PeekAsImpl(internal::multi_byte) const
    {
        static_assert(sizeof(F) > 1, "Require sizeof(F) > 1");
        static_assert(std::numeric_limits<F>::is_iec559, "IEEE 754 spec is enforced");
        using store_type = typename internal::floating_point_store_type<F>::type;
        auto as_binary = PeekAsImpl<store_type>(internal::multi_byte{});
        F f = *reinterpret_cast<F*>(&as_binary);
        return f;
    }

private:
    std::vector<value_type> buf_;
    size_t reader_index_;
    size_t writer_index_;
};

using Iterator = Buffer::Iterator;

inline Iterator operator+(Iterator it, Iterator::difference_type n)
{
    auto temp(it);
    temp += n;
    return temp;
}

inline Iterator operator+(Iterator::difference_type n, Iterator it)
{
    return it + n;
}

inline Iterator operator-(Iterator it, Iterator::difference_type n)
{
    auto temp(it);
    temp -= n;
    return temp;
}

inline bool operator<(Iterator lhs, Iterator rhs)
{
    return (rhs - lhs) > 0;
}

inline bool operator>(Iterator lhs, Iterator rhs)
{
    return rhs < lhs;
}

inline bool operator<=(Iterator lhs, Iterator rhs)
{
    return !(lhs > rhs);
}

inline bool operator>=(Iterator lhs, Iterator rhs)
{
    return !(lhs < rhs);
}

#if defined(OS_POSIX)

ssize_t ReadFDInVec(int fd, Buffer& buf);

#endif

}   // namespace ezio

#endif  // EZIO_BUFFER_H_
