#include <iostream>
#include <fstream>
#include "jsoncons/json.hpp"
#include "tclap/CmdLine.h"


int main(int argc, char* argv[])
{
    std::string filename, colIndexStr;

    TCLAP::CmdLine cmd("Extract one or more columns from simulation output json", ' ', "0.1");

    TCLAP::ValueArg<std::string> input_filename_arg("f", "file", "Input filepath", true, filename, "string");
    TCLAP::ValueArg<std::string> col_index_arg("i", "indexes", "comma-seperated column indexes to "
                                               "extract", true, colIndexStr, "string");
    
    cmd.add(input_filename_arg);
    cmd.add(col_index_arg);

    cmd.parse(argc, argv);

    filename = input_filename_arg.getValue();
    colIndexStr = col_index_arg.getValue();

    std::stringstream ss(colIndexStr);
    std::vector<int> indexes;
   
    for (long int i; ss >> i;) {
        indexes.push_back(i);

        if (ss.peek() == ',') {
            ss.ignore();
        }
    }

    std::ifstream is(filename);
    std::string toReturn;

    jsoncons::json simOutJson = jsoncons::json::parse(is);

    assert(indexes.size() != 0);

    for (const auto idx : indexes) {
        for (const auto& location : simOutJson["locations"].array_range()) {
            toReturn += (location[idx].as<std::string>() + ',');
        }
        toReturn.pop_back(); // remove last comma (,)

        toReturn += '\n';
    }
    
    toReturn.pop_back(); // remove newline
    std::cout << toReturn;

    return 0;
}

        

    
