#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"


#ifndef MEMORYMAP_FIXEDSTRING_HPP
#define MEMORYMAP_FIXEDSTRING_HPP

#include <cstring>
#include <cassert>
#include <stdexcept>

template<std::size_t N>
struct fixed_string
{
    char data_[N]{};

    constexpr fixed_string () noexcept = default;

    template<std::size_t M>
    constexpr fixed_string (const char (& data)[M]) noexcept
    {
        if constexpr (M > N + 1) throw std::range_error("String size bigger than buffer");

        constexpr auto BOUND = N < M ? N : M;
        for (int i = 0; i < BOUND; ++i) data_[i] = data[i];
    }

    template<std::size_t M>
    constexpr explicit fixed_string (fixed_string<M> const& x) noexcept
    {
        if constexpr (M > N + 1) throw std::range_error("String size bigger than buffer");
        constexpr auto BOUND = N < M ? N : M;
        for (int i = 0; i < BOUND; ++i) data_[i] = x.data_[i];
    }

    constexpr bool operator == (const char* rhs) const noexcept
    {
        return base_cmp(rhs) == 0;
    }

    constexpr bool operator != (const char* rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr bool operator < (const char* rhs) const noexcept
    {
        return base_cmp(rhs) < 0;
    }

    constexpr bool operator <= (const char* rhs) const noexcept
    {
        return base_cmp(rhs) <= 0;
    }

    constexpr bool operator > (const char* rhs) const noexcept
    {
        return !(*this <= rhs);
    }

    constexpr bool operator >= (const char* rhs) const noexcept
    {
        return !(*this < rhs);
    }

    template<std::size_t M>
    constexpr bool operator == (fixed_string<M> const& rhs) const noexcept
    {
        return base_cmp(rhs) == 0;
    }

    template<std::size_t M>
    constexpr bool operator != (fixed_string<M> const& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template<std::size_t M>
    constexpr bool operator < (fixed_string<M> const& rhs) const noexcept
    {
        return base_cmp(rhs) < 0;
    }

    template<std::size_t M>
    constexpr bool operator <= (fixed_string<M> const& rhs) const noexcept
    {
        return base_cmp(rhs) <= 0;
    }

    template<std::size_t M>
    constexpr bool operator > (fixed_string<M> const& rhs) const noexcept
    {
        return !(*this <= rhs);
    }

    template<std::size_t M>
    constexpr bool operator >= (fixed_string<M> const& rhs) const noexcept
    {
        return !(*this < rhs);
    }

    [[nodiscard]]
    constexpr const char* data () const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr std::size_t size () const noexcept
    {
        return std::strlen(data_);
    }

    constexpr char& operator [] (std::size_t position) noexcept
    {
        if (!(position < N)) std::terminate();
        return data_[position];
    }

    [[nodiscard]]
    constexpr char& operator [] (std::size_t position) const noexcept
    {
        if (!(position < N)) std::terminate();
        return data_[position];
    }

    constexpr char& at (std::size_t position)
    {
        if (!(position < N)) throw std::out_of_range("Out of range");
        return data_[position];
    }

    [[nodiscard]]
    constexpr char& at (std::size_t position) const
    {
        if (!(position < N)) throw std::out_of_range("Out of range");
        return data_[position];
    }

    [[nodiscard]]
    constexpr explicit operator bool () const noexcept
    {
        return data_[0];
    }

    [[nodiscard]]
    constexpr char* begin () noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const char* begin () const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr char* end () noexcept
    {
        return data_ + N;
    }

    [[nodiscard]]
    constexpr const char* end () const noexcept
    {
        return data_ + N;
    }

    [[nodiscard]]
    constexpr const char* cbegin () const noexcept
    {
        return data_;
    }

    [[nodiscard]]
    constexpr const char* cend () const noexcept
    {
        return data_ + N;
    }

private:
    constexpr auto base_cmp (const char* rhs) const
    {
        auto first1 = begin();
        auto first2 = rhs;

        for (;
            (*first1 != '\0') && (*first2 != '\0'); ++first1, (void) ++first2)
        {
            if (*first1 < *first2) return -1;
            if (*first2 < *first1) return 1;
        }
        return (*first1 == '\0') ? ((*first2 != '\0') ? -1 : 0) : 1;
    }

    template<std::size_t M>
    constexpr auto base_cmp (fixed_string<M> const& rhs) const
    {
        return base_cmp(rhs.begin());
    }
};

template<std::size_t N>
std::ostream& operator << (std::ostream& os, const fixed_string<N>& str)
{
    os << str.data();
    return os;
}

#endif //MEMORYMAP_FIXEDSTRING_HPP

#pragma clang diagnostic pop