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

template<typename T , template<class >  class IndexType>
void test(bool seq){
    int size = 100;
    std::cout<<"select test size: 100,200,500,1000,1500\n>";
    std::cin>>size;
    std::string file;
    switch (size) {
        case (100):
            file = "test100.csv";
            break;
        case(200):
            file = "test200.csv";
            break;
        case(500):
            file = "test500.csv";
            break;
        case(1000):
            file = "test1000.csv";
            break;
        case(1500):
            file = "winequality-red.csv";
            break;
        default: std::cout<<"not valid size";

    }
    std::cout<<"testing Hash:\n";
    std::cout<<"inserting to Hash:\n";
    readCsv<T,IndexType>(file,seq);
    std::cout<<"Index of size " +std::to_string(size) + " created\n";



    IndexType<T> hi("index", Pointer<>::WTE_FILE);
    for(int i = 1; i<=5;i ++){
        Sw::ScopedStopwatch<stopwatch::microseconds> sw("test add");
        bool b = hi.push({ size + i, 7.4, 0.7, 0.0, 1.9, 0.076, 11.0, 34.0, 0.9978, 3.51, 0.56, 9.4});
    }

    for(int i = 1; i<=5;i ++){
        Sw::ScopedStopwatch<stopwatch::microseconds> sw("test select");
       hi.find(i + size);
    }

    for(int i = 1; i<=5;i ++){
        Sw::ScopedStopwatch<stopwatch::microseconds> sw("test delete");
        hi.pop(i + size);
    }



}

void query_menu(){
    std::string buffer;
    std::cout<<"insertar un query:\n";
    std::cout<<">";
    std::cin.get();
    std::getline(std::cin,buffer);

//    parsersql("select 0 using index hash");
    parsersql(buffer);

}


void menu()
{

    std::cout<<"PROYECTO BASE DE DATOS II\n\n";
    while (true)
    {
        std::cout<<"1. Insertar un query\n";
        std::cout<<"2. Probar con queries existentes\n";
        std::cout<<"3. Salir del programa\n";
        std::cout<<"> ";
        int option;
        std::cin>>option;
        switch (option)
        {
            case 1:
                query_menu();
                break;
            case 2:
                std::cout<<"select index type\n1.Hash\n2.sequential\n";
                std::cout<<">";
                int indextype;
                std::cin>>indextype;
                switch(indextype){
                    case 1:
                        test<wineQuality,HashIndex>(false);
                        break;
                    case 2:
                        test<wineQuality,SeqIndex>(true);
                        break;
                    default:
                        std::cout<<"Insert a valid index\n";
                        break;
                }
                break;
            case 3:
                std::cout<<">> Cerrando programa...\n";
                return;
            default:
                std::cout<<"Por favor ingrese una opción válida.\n";
                break;
        }

    }
}




int main (int argc, char* argv[])
{
    _chdir("..");
    constexpr char name[] = "seq_files/index";
    constexpr char aux_name[] = "seq_files/aux_file";

    menu();

//    parsersql(
//        "insert into table Order from file ('test100.csv') using index hash");

//    parsersql(
//        "select 384 : 1008 using index hash");

}