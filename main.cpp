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
    parsersql(
        "select 384 : 1008 using index hash");

}