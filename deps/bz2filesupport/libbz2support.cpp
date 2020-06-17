#include "libbz2support.hpp"


bz2fileReaderWriter* bz2fileReaderWriter::instance_;
bio::filtering_istreambuf bz2fileReaderWriter::in;
bio::filtering_ostream bz2fileReaderWriter::out;


bz2fileReaderWriter* bz2fileReaderWriter::getInstance() {
    if (!instance_) {
        instance_ = new bz2fileReaderWriter();
    }

    return instance_;
}

void bz2fileReaderWriter::writebz2file(const std::string& filename, const std::string& toprint) {
    static bool init = false;

    if (!init) {
        out.push(bio::bzip2_compressor());
        out.push(bio::file_sink(filename.c_str(), std::ios::out | std::ios::binary)); // | std::ios::app));

        init = true;
    }

    out << toprint;

    out.flush(); // TODO check does it work?

}

std::unique_ptr<std::istream> bz2fileReaderWriter::readbz2file(const std::string& filename)
{
    in.push(bio::bzip2_decompressor());
    in.push(bio::file_descriptor_source(filename.c_str())); // OK

    // TODO handle exception
    std::unique_ptr <std::istream> istr(new std::istream(&in));

    return istr;
}

bz2fileReaderWriter::~bz2fileReaderWriter() {
    // TODO check if out was initialized
    out.pop();
}
