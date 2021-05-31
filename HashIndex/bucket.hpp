//
// Created by esteb on 28-May-21.
//

#ifndef MEMORYMAP_BUCKET_HPP
#define MEMORYMAP_BUCKET_HPP

#include "../headers/utils.hpp"
#include "../headers/pointer.tcc"
#include <algorithm>



template<class Record>
struct Bucket
{
    using Mode = Pointer<>::Mode;
    using Name = fixed_string<32>;
    using Key = decltype(get_key(std::declval<const Record&>()));

    struct Header
    {
        Pointer<Record> data;
        std::size_t size = 0;
        std::size_t depth = 0;
    };
    Header header;

    explicit
    Bucket (Name name, std::size_t depth = 0, Mode mode = Pointer<>::WTE_FILE)
    {
        if (mode == Pointer<>::CTE_FILE)
            create_bucket_file(name, depth);
        else
            load_from_memory(name);

    }

    void load_from_memory (const Name& name)
    {
        Pointer<Header> headerPtr(name, 0);
        header = *headerPtr;
    }

    void create_bucket_file (Name& name, std::size_t depth)
    {
        Pointer<Header> headerPtr(name, 0);
        { std::fstream tmp(name.data(), Pointer<>::CTE_FILE); }
        header.data = Pointer<Record>(name, sizeof(header));
        header.size = 0;
        header.depth = depth;
        headerPtr.set(header);
    }

    [[nodiscard]] auto begin () const noexcept
    { return header.data; }

    [[nodiscard]] auto end () const noexcept
    { return begin() + size(); }

    [[nodiscard]] auto size () const noexcept
    {
        return header.size;
    }

    [[nodiscard]] auto data () const noexcept
    {
        return header.data;
    }


    bool push_unsafe (const Record& record)
    {
        auto key = get_key(record);
        auto eq = [&key] (const auto& it) { return get_key(it) == key; };

        if (std::find_if(begin(), end(), eq) != end()) return false;

        this->end().set(record);

        set_size(size() + 1);
        return true;
    }

    // Only call if it's empty
    bool push_unsafe_on_empty (const Record* record_arr, int size)
    {
        auto stream = this->end().stream();
        stream.write((char*)(void*)record_arr, sizeof(Record) * size);
        set_size(size);
        return true;
    }

    bool pop_unsafe (const Key key)
    {
        auto eq = [&key] (const auto& it) { return get_key(it) == key; };
        auto it = std::find_if(begin(), end(), eq);
        if (it == end()) return false;

        it.set(*(end() - 1));

        set_size(size() - 1);
        return true;
    }

    void clear ()
    {
        set_size(0);
//        std::filesystem::resize_file(
//            header.data.filePath.data(), sizeof(Header));
    }

    void set_size (std::size_t s)
    {
        Pointer<std::size_t> ptr(data().filePath, offsetof(Header, size));
        ptr.set(s);
        header.size = s;
    }

    void set_depth (std::size_t depth)
    {
        Pointer<std::size_t> ptr(data().filePath, offsetof(Header, depth));
        ptr.set(depth);
        header.depth = depth;
    }
};


template<class Record>
[[nodiscard]] auto make_bucket (std::size_t depth)
{
    auto name = random_name();
    Bucket<Record> bucket(name, depth, Pointer<>::CTE_FILE);
    Pointer<Bucket<Record>> ptr(name, 0);
    return ptr;
}

#endif //MEMORYMAP_BUCKET_HPP
