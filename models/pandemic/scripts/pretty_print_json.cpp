#include <iostream>
#include <fstream>
#include "jsoncons/json.hpp"
#include "jsoncons/json.hpp"


int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    try {
        jsoncons::json json_text = jsoncons::json::parse(std::cin);

        std::ofstream outfile(argv[1], std::ios::out | std::ios::trunc);

        if (!outfile.is_open()) {
            std::cerr << "Error!" << std::endl;
            return 1;
        }

        jsoncons::json_options options;
        options.indent_size(2);

        outfile << jsoncons::pretty_print(json_text, options) << std::endl;

        outfile.close();

    } catch (const jsoncons::ser_error& e) {
        std::cerr << "Caught ser_error with category "
                  << e.code().category().name()
                  << ", code " << e.code().value()
                  << " and message " << e.what() << std::endl;

        return 1;
    }

    return 0;
}
