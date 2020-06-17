#ifndef LIBBZ2SUPPORT_HPP
#define LIBBZ2SUPPORT_HPP

#include <iostream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/device/file.hpp>

namespace bio = boost::iostreams;

#define BZ2FILE bz2fileReaderWriter::getInstance()


class bz2fileReaderWriter {

public:

    static bz2fileReaderWriter* getInstance();

    void writebz2file(const std::string& filename, const std::string& toprint);

    std::unique_ptr<std::istream> readbz2file(const std::string& filename);

    ~bz2fileReaderWriter();

private:

    bz2fileReaderWriter() = default;

    static bz2fileReaderWriter* instance_;

    static bio::filtering_istreambuf in;
    static bio::filtering_ostream out;
};


#endif
