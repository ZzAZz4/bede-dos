//
// Created by Jean Paul on 30/05/2021.
//

#ifndef MEMORYMAP_PARSER_H
#define MEMORYMAP_PARSER_H

#include "SeqIndex/seq_file.hpp"
#include "library.h"


const int BucketSize = 200;


template<typename T , template<class >  class IndexType>
void readCsv(const std::string& file = "test.csv"){
    std::fstream stream(file,std::ios::out | std::ios::in);
    IndexType<T> Index("index", Pointer<>::CTE_FILE);

    if(!stream.is_open()){
        std::cout<<"FILE NOT FOUND\n";
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::getline(stream, line); // skip the 1st line, you know why
    int index = 0;
    int disrupter = rand();
    int pushed = 0;
    while(std::getline(stream, line)){
        if (line.empty()) continue;// skip empty lines

        std::stringstream  iss(line);
        std::string line;


        wineQuality reg {};

        std::cout<<"index:"<<index<<'\n';
        reg.id = index * rand() % (1500 * 10);


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

//        debug line
//        std::cout<<reg<<'\n';

        if(!Index.push(reg)) pushed--;
        index++;
        pushed++;

    }
    std::cout<<"pushed"<<pushed<<"\n";

}


template<typename T , template<class >  class IndexType = HashIndex>
void IndexController(const std::string& query){
    IndexType<T> Index("index", Pointer<>::WTE_FILE);
    int findPos = query.find("file",0);

    if(findPos != std::string::npos){
        findPos = query.find("'",findPos);
        int findEnd = query.find("'", ++findPos);
        std::string filePath = query.substr(findPos, findEnd  - findPos);
        std::cout<<filePath<<'\n';
        readCsv<T,IndexType>(filePath);
    }
    else if( (findPos = query.find("select")) != std::string::npos){
        findPos+=6;
        std::stringstream  iss(query.substr(findPos));
        std::string buffer;
        iss>>buffer;

        if(buffer == "*"){
            Index.print();
        }
        else if(buffer != "from"){
            std::stringstream  iss_2(query.substr(findPos ));
            std::string rangeBuffer;
            decltype(get_key(std::declval<const wineQuality&>())) val;
            iss_2 >>val;
            iss_2>>rangeBuffer;
            if(rangeBuffer == ":"){
                decltype(get_key(std::declval<const wineQuality&>())) val2;
                iss_2 >> val2;
                Index.find( val,val2);

            }else std::cout<<Index.find( val).value()<<'\n';


        }
        else{
            std::cerr<<"did not specify any kind of key";
            exit(-1);
        }

    }
    else if((findPos = query.find("delete")) != std::string::npos){
        findPos+=6;
        std::stringstream  iss(query.substr(findPos));
        std::string buffer;
        iss>>buffer;


        if(buffer != "from"){
            std::stringstream  iss_2(query.substr(findPos ));
            decltype(get_key(std::declval<const wineQuality&>())) val;
            iss_2 >>val;

            Index.pop( val);
        }
        else{
            std::cerr<<"did not specify any kind of key";
            exit(-1);
        }
    }
    else{
        std::cerr<<"Operation not supported";
        exit(-1);
    }

}

void parsersql(std::string query){
    int usingPos = query.find("using",0);
    if(usingPos == std::string::npos) {std::cerr<<"Missing file structure in query" ; exit(-1) ; }


    std::string  s1 = query.substr(usingPos + 5);
    std::string buffer;
    std::string res;
    std::stringstream  iss(s1);

    iss >> buffer;
    res = buffer + " ";
    iss >> buffer;
    res += buffer;
    if(res == "index hash"){
        IndexController<wineQuality,HashIndex>(query.substr(0,usingPos - 1));
    }
    else if(res == "sequential file"){
        IndexController<wineQuality,SeqIndex>(query.substr(0,usingPos - 1));

    }
    else{
        std::cerr<<"file organization not supported  " ;
        exit(-1) ;
    }
}

#endif //MEMORYMAP_PARSER_H
