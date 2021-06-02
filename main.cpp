#include "Stopwatch.hpp"

namespace Sw = stopwatch;

#include "library.h"
#include "parser.h"
#include "SeqIndex/seq_file.hpp"

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
    constexpr char name[] = "seq_files/index";
    constexpr char aux_name[] = "seq_files/aux_file";

//    parsersql(
//        "insert into table Order from file ('test100.csv') using index hash");



//    parsersql("select 3 using index hash");
    SeqIndex<wineQuality> test(name, Pointer<>::CTE_FILE);
    std::cout << test.header.main_alloc_pos << '\n';
    test.push({0, 7.4,0.7,0.0,1.9,0.076,11.0,34.0,0.9978,3.51,0.56,9.4 });
    std::cout << test.header.main_alloc_pos << '\n';
    test.push({1, 7.4,0.7,0.0,1.9,0.076,11.0,34.0,0.9978,3.51,0.56,9.4 });
    std::cout << test.header.main_alloc_pos << '\n';
    test.push({2, 7.4,0.7,0.0,1.9,0.076,11.0,34.0,0.9978,3.51,0.56,9.4 });
    std::cout << test.header.main_alloc_pos << '\n';
    test.push({10, 7.4,0.7,0.0,1.9,0.076,11.0,34.0,0.9978,3.51,0.56,9.4 });
    std::cout << test.header.main_alloc_pos << '\n';
    test.push({4, 7.4,0.7,0.0,1.9,0.076,11.0,34.0,0.9978,3.51,0.56,9.4 });
    std::cout << test.header.main_alloc_pos << '\n';

    test.print();

    std::cout << std::endl;

    test.pop(0);
    test.push({0, 7.4,0.7,0.0,1.9,0.076,11.0,34.0,0.9978,3.51,0.56,9.4 });

    test.print();







//    for (auto i : test.find(0, 10))
//    {
//        std::cout << i << '\n';
//    }
//    assert(!test.find(5).has_value());
//    test.push(5);
//    assert(test.find(5).has_value());

//    {
//        HashIndex<Record, 3> hashIndex("index", Pointer<>::CTE_FILE);
//        hashIndex.push(Record{ 1, MALE, "peto" });
//        hashIndex.push(Record{ 1, FEMALE, "XD" });
//        hashIndex.push(Record{ 2, MALE, "pato" });
//        hashIndex.push(Record{ 4, FEMALE, "pata" });
//        hashIndex.push(Record{ 6, MALE, "umu" });
//
//        hashIndex.print();
//    }
//    {
//        HashIndex<Record, 3> hashIndex("index", Pointer<>::WTE_FILE);
//        auto val = hashIndex.find(4);
//        assert(val.has_value());
//        std::cout << val.value() << "\n\n";
//        assert(!hashIndex.find(7).has_value());
//    }
//    {
//        HashIndex<Record, 3> hashIndexCopy("index", Pointer<>::WTE_FILE);
//        hashIndexCopy.print();
//
//        auto val = hashIndexCopy.find(6);
//        assert(val.has_value());
//        std::cout << val.value() << "\n\n";
//        assert(!hashIndexCopy.find(7).has_value());
//
//        hashIndexCopy.pop(1);
//        hashIndexCopy.pop(4);
//        hashIndexCopy.print();
//    }
//    Sw::ScopedStopwatch sw("parser");
//    parsersql(
//        "insert into table Order from file ('winequality-red.csv') using index hash");
//        parsersql("select * from Order using index hash");
//        parsersql("select 2 from Order using index hash");
//        parsersql("delete 2 from Order using index hash");
//        parsersql("select * from Order using index hash");
//        std::string query ;
//        while (true){
//           std::getline(std::cin,query);
//           parsersql(query);
//        }

    int size = 100;
    HashIndex<wineQuality> hashIndexCopy("index", Pointer<>::WTE_FILE);
    for(int i = 0; i < 5; i ++) {
        Sw::ScopedStopwatch<stopwatch::microseconds> sw("test");
//        if (hashIndexCopy.push({ (i+1) * size + 5, 7.4, 0.7, 0.0, 1.9, 0.076, 11.0, 34.0, 0.9978, 3.51, 0.56, 9.4}))
//            std::cout << "A ";
//        if(hashIndexCopy.find(i * size ).has_value() ) std::cout<<"A ";
    }
    hashIndexCopy.print();



//    readCsv();
//
//        {
//        HashIndex<wineQuality, 10> hashIndexCopy("index", Pointer<>::WTE_FILE);
//        hashIndexCopy.print();
//
//        auto val = hashIndexCopy.find(0);
//        assert(val.has_value());
//        std::cout << val.value() << "\n\n";
//        assert(!hashIndexCopy.find(7).has_value());
//
//        hashIndexCopy.pop(1);
//        hashIndexCopy.pop(2);
//        hashIndexCopy.print();
//    }


}