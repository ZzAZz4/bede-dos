#include <iostream>
#include <direct.h>
#include <cassert>
#include <sstream>
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

struct wineQuality{
    int id;
    float fixed_acidity;
    float citric_acidity;
    float residual_sugar;
    float chlorides;
    float free_sulfur_dioxide;
    float total_sulfur_dioxide;
    float density;
    float ph;
    float sulphates;
    float alcohol;
    float quality;

    [[nodiscard]]
    const auto& key () const
    {
        return id;
    }
};

std::ostream& operator << (std::ostream& os, const wineQuality& r)
{
    os << r.id << ' '
       << r.fixed_acidity<<' '
       << r.citric_acidity<<' '
       << r.residual_sugar<<' '
       << r.chlorides<<' '
       << r.free_sulfur_dioxide<<' '
            ;
    return os;
}



void readCsv(const std::string& file = "winequality-red.csv"){

    std::fstream stream(file, std::ios::binary | std::ios::out | std::ios::in);
    HashIndex<wineQuality, 3> hashIndex("index", Pointer<>::CTE_FILE);

    if(!stream.is_open()){
        std::cout<<"FILE NOT FOUND\n";
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::getline(stream, line); // skip the 1st line, you know why
    int index = 0;
    while(std::getline(stream, line)){
        if (line.empty()) continue;// skip empty lines

        std::stringstream  iss(line);
        std::string line;


        wineQuality reg {};

        std::cout<<"index:"<<index<<'\n';
        reg.id = index;


        iss>>reg.fixed_acidity;
        std::getline(iss, line, ',');
        iss>>reg.citric_acidity;
        std::getline(iss, line, ',');
        iss>>reg.residual_sugar;
        std::getline(iss, line, ',');
        iss>>reg.chlorides;
        std::getline(iss, line, ',');
        iss>>reg.free_sulfur_dioxide;
        std::getline(iss, line, ',');
        iss>>reg.total_sulfur_dioxide;
        std::getline(iss, line, ',');
        iss>>reg.density;
        std::getline(iss, line, ',');
        iss>>reg.ph;
        std::getline(iss, line, ',');
        iss>>reg.sulphates;
        std::getline(iss, line, ',');
        iss>>reg.alcohol;
        std::getline(iss, line, ',');
        iss>>reg.quality;

        //debug line
//        std::cout<<reg<<'\n';

        hashIndex.push(reg);
        index++;

    }

}

int main (int argc, char* argv[])
{
    _chdir("..");

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

    readCsv();

        {
        HashIndex<wineQuality, 100> hashIndexCopy("index", Pointer<>::WTE_FILE);
        hashIndexCopy.print();

        auto val = hashIndexCopy.find(0);
        assert(val.has_value());
        std::cout << val.value() << "\n\n";
        assert(!hashIndexCopy.find(7).has_value());

        hashIndexCopy.pop(1);
        hashIndexCopy.pop(2);
//        hashIndexCopy.print();
    }

}