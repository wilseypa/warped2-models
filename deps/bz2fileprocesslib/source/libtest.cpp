#include <iostream>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

namespace bio = boost::iostreams;

std::unique_ptr<std::istream> readbz2file(std::string filename)
{
    static bio::filtering_istreambuf in;

    in.push(bio::bzip2_decompressor()); 

    in.push(bio::file_descriptor_source(filename.c_str())); // OK

    // TODO handle exception
    std::unique_ptr <std::istream> in_config(new std::istream(&in));

    // if (*in_config) std::cout << "istream created!" << std::endl;

    return in_config;
}

// TODO add function for writing compressed file



