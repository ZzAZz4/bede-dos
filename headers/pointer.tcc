//
// Created by esteb on 21-May-21.
//
#ifndef MEMORYMAP_POINTER_TCC
#define MEMORYMAP_POINTER_TCC


#include <fstream>
#include <utility>
#include <array>
#include <limits>
#include "fixed_string.hpp"

template<class T>
constexpr long sizeOf ()
{
    return sizeof(T);
}

template<>
constexpr long sizeOf<void> ()
{
    return 1;
}

template<class T = void, std::size_t NAME_LEN = 32>
struct Pointer;

template<class T = void, std::size_t NAME_LEN = 32>
struct ReversePointer;

template<template<class T, std::size_t NAME_LEN> class DerivedTmpl, class T, std::size_t NAME_LEN, long VALUE_SZ_ = sizeOf<T>()>
struct PointerImpl
{
    using Derived = DerivedTmpl<T, NAME_LEN>;

    using Value = T;
    using Path = fixed_string<NAME_LEN>;
    using Stream = std::fstream;
    using Position = long;
    using Mode = std::ios::openmode;

    static constexpr auto VALUE_SZ = VALUE_SZ_;

    using iterator_category = std::random_access_iterator_tag;
    using difference_type = Position;
    using value_type = std::conditional<std::is_same_v<Value, void>, char, Value>;
    using pointer = value_type*;
    using reference = value_type&;

    static constexpr auto RDO_FILE = std::ios::in | std::ios::binary;
    static constexpr auto WTE_FILE = RDO_FILE | std::ios::out;
    static constexpr auto CTE_FILE = std::ios::out | std::ios::binary;
    static constexpr auto NPOS = LONG_MAX;


    Path filePath;
    Position position;

    constexpr explicit
    PointerImpl (Path path = Path(), Position pos = NPOS)
        : filePath(path), position(pos)
    {
    }

    [[nodiscard]]
    Stream stream () const noexcept
    {
        std::fstream stream(this->filePath.data(), WTE_FILE);
        stream.seekg(position);
        return stream;
    }

    [[nodiscard]]
    Stream cstream () const noexcept
    {
        std::fstream stream(this->filePath.data(), RDO_FILE);
        stream.seekg(position);
        return stream;
    }

    template<class U, std::size_t N>
    constexpr
    bool operator == (const DerivedTmpl<U, N>& other) const noexcept
    {
        return this->filePath == other.filePath &&
               this->position == other.position;
    }

    template<class U, std::size_t N>
    constexpr
    bool operator != (const DerivedTmpl<U, N>& other) const noexcept
    {
        return !(*this == other);
    }

    constexpr explicit
    operator bool () const noexcept
    {
        return this->filePath == "" && position == NPOS;
    }

    constexpr
    Derived operator + (Position offset) const noexcept
    {
        return Derived(this->filePath, this->position + VALUE_SZ * offset);
    }

    constexpr Derived operator - (Position offset) const noexcept
    {
        return Derived(this->filePath, this->position - VALUE_SZ * offset);
    }

    Derived& operator ++ () noexcept
    {
        this->position += VALUE_SZ;
        return *reinterpret_cast<Derived*>(this);
    }

    Derived& operator += (difference_type offset) noexcept
    {
        this->position += VALUE_SZ * offset;
        return *reinterpret_cast<Derived*>(this);
    }

    Derived& operator -= (difference_type offset) noexcept
    {
        this->position -= VALUE_SZ * offset;
        return *reinterpret_cast<Derived*>(this);
    }

    Derived& operator -- () noexcept
    {
        this->position -= VALUE_SZ;
        return *reinterpret_cast<Derived*>(this);
    }

    Derived operator ++ (int) noexcept
    {
        Derived tmp = *(Derived*) this;
        ++(*this);
        return tmp;
    }

    Derived operator -- (int) noexcept
    {
        Derived tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr difference_type operator - (const Derived& other) const
    {
        if (this->filePath != other.filePath)
            throw std::bad_cast();

        return (this->position - other.position) / VALUE_SZ;
    }

    auto operator [] (Position index) const
    {
        return *(*this + index);
    }
};

template<template<class T, std::size_t NAME_LEN> class DerivedTmpl, class T, std::size_t NAME_LEN, long VALUE_SZ_ = sizeOf<T>()>
struct RevPointerImpl : PointerImpl<DerivedTmpl, T, NAME_LEN, -VALUE_SZ_>
{
    using Base = PointerImpl<DerivedTmpl, T, NAME_LEN, -VALUE_SZ_>;
    using Path = typename Base::Path;
    using Position = typename Base::Position;
    using Mode = typename Base::Mode;

    using Base::WTE_FILE;
    using Base::CTE_FILE;
    using Base::RDO_FILE;
    using Base::NPOS;

    using iterator_category = typename Base::iterator_category;
    using difference_type = typename Base::difference_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    constexpr explicit
    RevPointerImpl (Path path = Path(), Position pos = NPOS)
        : Base(path, pos)
    {}
};

template<class T, std::size_t NAME_LEN>
struct Pointer : PointerImpl<Pointer, T, NAME_LEN>
{
    using Base = PointerImpl<Pointer, T, NAME_LEN>;
    using Path = typename Base::Path;
    using Position = typename Base::Position;
    using Mode = typename Base::Mode;

    using Base::WTE_FILE;
    using Base::CTE_FILE;
    using Base::RDO_FILE;
    using Base::NPOS;

    using iterator_category = typename Base::iterator_category;
    using difference_type = typename Base::difference_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    constexpr explicit
    Pointer (Path path = Path(), Position pos = NPOS)
        : Base(path, pos)
    {}

    [[nodiscard]]
    T get () const noexcept
    {
        auto stream = this->cstream();
        char buffer[sizeof(T)];
        stream.read(buffer, sizeof(T));
        return *(T*) (void*) buffer;
    }

    void set (const T& value) const noexcept
    {
        auto stream = this->stream();
        assert(stream.is_open());
        stream.write((char*) (void*) &value, sizeof(T));
    }

    operator Pointer<void, NAME_LEN> () const
    {
        return Pointer<void, NAME_LEN>(this->filePath, this->position);
    }

    template<class U>
    explicit operator Pointer<
        std::enable_if < std::is_convertible<T, U>::value, U>, NAME_LEN
    > () const
    {
        return Pointer<U, NAME_LEN>(this->filePath, this->position);
    };

    constexpr auto makeReverse() const
    {
        return ReversePointer<T, NAME_LEN>(this->filePath, this->position);
    }

    T operator * () const
    {
        return get();
    }


};

template<std::size_t NAME_LEN>
struct Pointer<void, NAME_LEN> : PointerImpl<Pointer, void, NAME_LEN>
{
    using Base = PointerImpl<Pointer, void, NAME_LEN>;

    using iterator_category = typename Base::iterator_category;
    using difference_type = typename Base::difference_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    using Path = typename Base::Path;
    using Position = typename Base::Position;
    using Mode = typename Base::Mode;

    using Base::WTE_FILE;
    using Base::CTE_FILE;
    using Base::RDO_FILE;
    using Base::NPOS;

    constexpr explicit
    Pointer (Path path = Path(), Position pos = NPOS)
        : Base(path, pos)
    {}

    template<class T>
    operator Pointer<T, NAME_LEN> () const
    {
        return Pointer<T, NAME_LEN>(this->filePath, this->position);
    }

    constexpr auto makeReverse() const
    {
        return ReversePointer<void, NAME_LEN>(this->filePath, this->position);
    }
};

template<class T, std::size_t NAME_LEN>
struct ReversePointer : RevPointerImpl<ReversePointer, T, NAME_LEN>
{
    using Base = RevPointerImpl<ReversePointer, T, NAME_LEN>;
    using Path = typename Base::Path;
    using Position = typename Base::Position;
    using Mode = typename Base::Mode;

    using Base::WTE_FILE;
    using Base::CTE_FILE;
    using Base::RDO_FILE;
    using Base::NPOS;

    using iterator_category = typename Base::iterator_category;
    using difference_type = typename Base::difference_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    constexpr explicit
    ReversePointer(Path path = Path(), Position pos = NPOS)
        : Base(path, pos)
    {}

    [[nodiscard]]
    T get () const noexcept
    {
        auto stream = this->cstream();
        char buffer[sizeof(T)];
        stream.read(buffer, sizeof(T));
        return *(T*) (void*) buffer;
    }

    void set (const T& value) const noexcept
    {
        auto stream = this->stream();
        assert(stream.is_open());
        stream.write((char*) (void*) &value, sizeof(T));
    }

    operator ReversePointer<void, NAME_LEN> () const
    {
        return Pointer<void, NAME_LEN>(this->filePath, this->position);
    }

    template<class U>
    explicit operator ReversePointer<
        std::enable_if < std::is_convertible<T, U>::value, U>, NAME_LEN
    > () const
    {
        return Pointer<U, NAME_LEN>(this->filePath, this->position);
    };

    T operator * () const
    {
        return get();
    }

    constexpr auto makePointer() const
    {
        return Pointer<T, NAME_LEN>(this->filePath, this->position);
    }
};


template<std::size_t NAME_LEN>
struct ReversePointer<void, NAME_LEN> : RevPointerImpl<ReversePointer, void, NAME_LEN>
{
    using Base = RevPointerImpl<ReversePointer, void, NAME_LEN>;

    using iterator_category = typename Base::iterator_category;
    using difference_type = typename Base::difference_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    using Path = typename Base::Path;
    using Position = typename Base::Position;
    using Mode = typename Base::Mode;

    using Base::WTE_FILE;
    using Base::CTE_FILE;
    using Base::RDO_FILE;
    using Base::NPOS;

    constexpr explicit
    ReversePointer (Path path = Path(), Position pos = NPOS)
        : Base(path, pos)
    {}

    template<class T>
    operator ReversePointer<T, NAME_LEN> () const
    {
        return Pointer<T, NAME_LEN>(this->filePath, this->position);
    }

    constexpr auto makePointer() const
    {
        return Pointer<void, NAME_LEN>(this->filePath, this->position);
    }
};

#endif //MEMORYMAP_POINTER_TCC