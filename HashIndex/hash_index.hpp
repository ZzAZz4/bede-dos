//
// Created by esteb on 28-May-21.
//

#ifndef MEMORYMAP_HASH_INDEX_HPP
#define MEMORYMAP_HASH_INDEX_HPP

#include "bucket.hpp"
#include <optional>
#include <vector>


template<class Record,
    class Hash = std::hash<decltype(get_key(std::declval<const Record&>()))>>
struct HashIndex
{
    using Key = decltype(get_key(std::declval<const Record&>()));
    using Bucket = Bucket<Record>;
    using Mode = Pointer<>::Mode;
    using Name = fixed_string<32>;

    int N;

    inline static Hash hash_base = Hash();

    struct Directory
    {
        Pointer<Pointer<Bucket>> data;
        std::size_t size = 1;
        std::size_t depth = 0;
        std::size_t active_on_depth = 1;
    };

    Directory buckets;

    explicit HashIndex (Name name, Mode mode = Pointer<>::WTE_FILE ,  int BucketSize = 200):N(BucketSize)
    {
        if (mode == Pointer<>::CTE_FILE)
            create_index_file(name);
        else
            load_from_memory(name);
    }

    bool push (const Record& record)
    {
        auto index = hash(get_key(record));
        auto bucket = *buckets.data[index];
        auto success = bucket.push_unsafe(record);

        if (success && bucket.size() > N)
        {
            if (bucket.header.depth == buckets.depth)
                extend_table();
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
            const int index_to_merge = index & ~(1 << (buckets.depth - 1));
            merge_into(index_to_merge);
            if (!buckets.active_on_depth)
                shrink_table();
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

        for (int i = buckets.size; i < 2 * buckets.size; ++i)
            (buckets.data + i).set(buckets.data[i - buckets.size]);

        buckets.depth++, buckets.size *= 2, buckets.active_on_depth = 0;
        ptr.set(buckets);
    }

    void shrink_table ()
    {
        Pointer<Directory> ptr(this->buckets.data.filePath, 0);
        buckets.depth--, buckets.size /= 2, buckets.active_on_depth = 0;
        for (auto i : *this)
        {
            if ((*i).header.depth == buckets.depth)
                ++buckets.active_on_depth;
        }
        ptr.set(buckets);
    }

    void split_at (int index)
    {
        auto newIndex = index | 1 << (buckets.depth - 1);

        auto curBucket = *buckets.data[index];
        curBucket.set_depth(curBucket.header.depth + 1);
        auto newBucketPtr = make_bucket<Record>(curBucket.header.depth);
        (buckets.data + newIndex).set(newBucketPtr);
        auto newBucket = *newBucketPtr;

        auto entries = std::vector<Record>(curBucket.begin(), curBucket.end());
        curBucket.clear();
        auto cond = [=] (const auto& i) { return hash(get_key(i)) == index; };
        auto split = std::partition(entries.begin(), entries.end(), cond);

        curBucket.push_unsafe_on_empty(entries.begin().base(), std::distance(entries.begin(), split));
        newBucket.push_unsafe_on_empty(split.base(), std::distance(split, entries.end()));

        set_active(buckets.active_on_depth + 2);
    }

    void merge_into (int index)
    {
        auto other = index | 1 << (buckets.depth - 1);

        auto cur = buckets.data + index;
        auto oth = buckets.data + other;

        if (auto tmp = *cur; (*tmp).header.size == 0)
            cur.set(*oth);
        else
            oth.set(tmp);
        (**cur).set_depth((**cur).header.depth - 1);
        set_active(buckets.active_on_depth - 2);
    }


    [[nodiscard]] auto begin () const noexcept
    { return buckets.data; }

    [[nodiscard]] auto size () const noexcept
    { return buckets.size; }

    [[nodiscard]] auto end () const noexcept
    { return begin() + size(); }

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

private:
    [[nodiscard]]
    auto hash (const Key& key) const
    {
        return hash_base(key) % (1 << buckets.depth);
    }

    void create_index_file (Name& name)
    {
        Pointer<Directory> directory_ptr(name, 0);
        {
            volatile std::fstream tmp(name.data(), Pointer<>::CTE_FILE);
        }
        buckets = Directory{
            .data = Pointer<Pointer<Bucket>>(name, sizeof(buckets)) };

        auto bucket = make_bucket<Record>(buckets.depth);
        directory_ptr.set(buckets);
        buckets.data.set(bucket);
    }

    void load_from_memory (Name& name)
    { buckets = *Pointer<Directory>(name, 0); }

    void set_active (std::size_t active)
    {
        Pointer<std::size_t> ptr(
            buckets.data.filePath, offsetof(Directory, active_on_depth));
        ptr.set(active);
        buckets.active_on_depth = active;
    }
};


#endif //MEMORYMAP_HASH_INDEX_HPP
