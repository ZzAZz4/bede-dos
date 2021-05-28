#include <iostream>
#include <direct.h>
#include <cassert>
#include "HashIndex/hash_index.hpp"

enum Gender
{
    MALE, FEMALE
};

struct Record
{
    int id;
    Gender gender;
    char name[16];

    // required for it to work
    [[nodiscard]]
    const auto& key () const
    {
        return id;
    }
};

std::ostream& operator << (std::ostream& os, const Record& r)
{
    os << r.id << ' '
       << (r.gender == MALE ? "MALE" : "FEMALE")
       << ' ' << r.name;
    return os;
}

int main (int argc, char* argv[])
{
    _chdir("..");

    {
        HashIndex<Record, 3> hashIndex("index", Pointer<>::CTE_FILE);
        hashIndex.push(Record{ 1, MALE, "peto" });
        hashIndex.push(Record{ 2, MALE, "pato" });
        hashIndex.push(Record{ 4, FEMALE, "pata" });
        hashIndex.push(Record{ 6, MALE, "umu" });

        hashIndex.print();
    }
    {
        HashIndex<Record, 3> hashIndex("index", Pointer<>::WTE_FILE);
        auto val = hashIndex.find(4);
        assert(val.has_value());
        std::cout << val.value() << "\n\n";
        assert(!hashIndex.find(7).has_value());
    }
    {
        HashIndex<Record, 3> hashIndexCopy("index", Pointer<>::WTE_FILE);
        hashIndexCopy.print();

        auto val = hashIndexCopy.find(6);
        assert(val.has_value());
        std::cout << val.value() << "\n\n";
        assert(!hashIndexCopy.find(7).has_value());

        hashIndexCopy.pop(1);
        hashIndexCopy.pop(4);
        hashIndexCopy.print();
    }

}