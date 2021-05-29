//
// Created by esteb on 28-May-21.
//

#ifndef MEMORYMAP_HASH_INDEX_HPP
#define MEMORYMAP_HASH_INDEX_HPP

#include "bucket.hpp"
#include <optional>
#include <vector>


template<class Record, int N,
    class Hash = std::hash<decltype(get_key(std::declval<const Record&>()))>>
struct HashIndex
{
    using Key = decltype(get_key(std::declval<const Record&>()));
    using Bucket = Bucket<Record>;
    using Mode = Pointer<>::Mode;
    using Name = fixed_string<8>;

    inline static Hash hash_base = Hash();

    struct Directory
    {
        Pointer<Pointer<Bucket>> data;
        std::size_t size{};
        std::size_t depth{};
        std::size_t active{};
    };

    Directory buckets;


    [[nodiscard]] auto begin () const noexcept
    { return buckets.data; }

    [[nodiscard]] auto size () const noexcept
    { return buckets.size; }

    [[nodiscard]] auto end () const noexcept
    { return begin() + size(); }

    explicit
    HashIndex (Name name, Mode mode = Pointer<>::WTE_FILE)
    {
        Pointer<Directory> directory_ptr(name, 0);
        if (mode == Pointer<>::CTE_FILE)
        {
            {
                std::fstream tmp(name.data(), mode);
            }
            buckets.data = Pointer<Pointer<Bucket>>(name, sizeof(buckets));
            buckets.size = 1;
            buckets.depth = 0;
            buckets.active = 1;

            auto bucket = make_bucket<Record>(buckets.depth);
            directory_ptr.set(buckets);
            buckets.data.set(bucket);
        }
        else buckets = *directory_ptr;
    }

    void reload ()
    {
        Pointer<Directory> directory_ptr(buckets.data.filePath, 0);
        buckets = *directory_ptr;
    }

    bool push (const Record& record)
    {
        auto index = hash(get_key(record));
        auto bucket = *buckets.data[index];
        auto success = bucket.push_unsafe(record);

        if (success && bucket.size() > N)
        {
            if (bucket.header.depth == buckets.depth) extend_table();
            split_at(index);
        }
        return success;
    }

    bool pop (const Key& key)
    {
        auto index = hash(key);
        auto bucket = *buckets.data[index];
        auto success = bucket.pop_unsafe(key);

        if (success && bucket.header.size == 0 && buckets.size > 1)
        {
            merge_at(index & ~(1 << (buckets.depth - 1)));
            if (!buckets.active) shrink_table();
        }
        return success;
    }

    std::optional <Record> find (const Key& key)
    {
        auto index = hash(key);
        auto bucket = *buckets.data[index];
        for (auto record : bucket)
        {
            if (get_key(record) == key)
                return record;
        }
        return std::nullopt;
    }

    void extend_table ()
    {
        Pointer<Directory> ptr(this->buckets.data.filePath, 0);
        buckets.depth++;
        auto size = buckets.size;
        buckets.size *= 2;
        for (int i = size; i < buckets.size; ++i)
        {
            (buckets.data + i).set(buckets.data[i - size]);
        }
        buckets.active = 0;
        ptr.set(buckets);
    }

    void shrink_table ()
    {
        Pointer<Directory> ptr(this->buckets.data.filePath, 0);
        buckets.depth--;
        buckets.size >>= 1;
        buckets.active = 0;
        for (auto i : *this)
        {
            if ((*i).header.depth == buckets.depth)
                ++buckets.active;
        }
        ptr.set(buckets);
    }

    void split_at (int index)
    {
        auto cond = [=] (const auto& i) { return hash(get_key(i)) == index; };
        auto newIndex = index | 1 << (buckets.depth - 1);

        auto curBucket = *buckets.data[index];
        curBucket.set_depth(curBucket.header.depth + 1);
        auto newBucketPtr = make_bucket<Record>(curBucket.header.depth);
        (buckets.data + newIndex).set(newBucketPtr);
        auto newBucket = *newBucketPtr;

        auto entries = std::vector<Record>(curBucket.begin(), curBucket.end());
        curBucket.clear();

        auto split = std::partition(entries.begin(), entries.end(), cond);
        for (auto it = entries.begin(); it != split; ++it)
        {
            curBucket.push_unsafe(*it);
        }
        for (auto it = split; it != entries.end(); ++it)
        {
            newBucket.push_unsafe(*it);
        }
        set_active(buckets.active + 2);
    }

    void merge_at (int index)
    {
        auto other = index | 1 << (buckets.depth - 1);

        auto cur = buckets.data + index;
        auto oth = buckets.data + other;

        if (auto tmp = *cur; (*tmp).header.size == 0)
            cur.set(*oth);
        else
            oth.set(tmp);
        (**cur).set_depth((**cur).header.depth - 1);
        set_active(buckets.active - 2);
    }

    void set_active (std::size_t active)
    {
        Pointer<std::size_t> ptr(
            buckets.data.filePath, offsetof(Directory, active));
        ptr.set(active);
        buckets.active = active;
    }

    void print () const
    {
        for (Pointer<Bucket> ptr : *this)
        {
            Bucket entry = *ptr;
            std::cout << entry.header.data.filePath << " -> ";
            std::cout << "(" << entry.header.depth << ")" << ' ';
            std::cout<<'\n';
            for (auto i : entry)
            {
                std::cout << i << '\n';
            }
            std::cout << '\n';
        }
    }

    [[nodiscard]] auto hash (const Key& key) const
    {
        return hash_base(key) % (1 << buckets.depth);
    }
};


#endif //MEMORYMAP_HASH_INDEX_HPP
