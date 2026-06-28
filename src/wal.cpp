#include <fstream>
#include <stdexcept>
#include "../include/wal.h"
#include <iostream>
#include "../src/crc32.cpp"
#include <filesystem>
#include <vector>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

bool wal::WAL::recover()
{
    // flow

    file_.seekg(0, ios::beg);

    // loop till whole file complete or reading fails
    while (true)
    {

        // we hv reached the eof
        RecordHeader header;
        file_.read(reinterpret_cast<char *>(&header), sizeof(header));
        if (!file_)
        {
            if (file_.eof())
            {
                file_.clear();
                return true;
            }
            else
                return false;
        }

        // read header
        uint32_t payload = header.payload_size;
        // read payload size
        vector<char> payload_data(payload);

        file_.read(payload_data.data(), payload);
        if (!file_ || file_.gcount() != payload)
        {
            return false;
        }
        // read payload if payload size isnt equal to payload size stop its corrputed

        uint32_t current_crc = calculate_crc(reinterpret_cast<const uint8_t *>(payload_data.data()), payload);

        if (current_crc != header.crc32)
        {
            return false;
        }
        string record(payload_data.begin(), payload_data.end());
        records_.push_back(record);
        cout << header.lsn << " " << record << endl;
        // recompute crc
        // it should mathc wiht the earlier crc if not stop
        next_lsn_ = header.lsn + 1;
    }

    return true;
}

bool wal::WAL::flush()
{
    file_.flush();
    // this pushes the data from temporary page to os cache but still not to disk
    // we need to force it to write to disk
    // no idea how to actually do it,how about i put fd in constructor only from the start
    //  i dropped that idea since i think i will hv to change append and other functions and i  am too lazy to do that
    // lets create another handle

    if (!file_)
        return false;

    int fd = _open(file_path_.c_str(), _O_WRONLY);//wronly opens for writing only we here force it to move from os to disk

    if (fd == -1)
        return false;

    bool ok = (_commit(fd) != -1);

        _close(fd);

    return ok;
}

wal::WAL::~WAL()
{

    flush();

    if (file_.is_open())
    {
        file_.close();
    }
}

wal::WAL::WAL(const string &file_path) : file_path_(file_path)
{

    file_.open(file_path_, ios::binary | ios::in | ios::out);

    if (!file_.is_open())
    {
        cout << "file doestn exists creating new file" << endl;
        ofstream create_file(file_path_, std::ios::binary);

        create_file.close();

        file_.open(file_path_, ios::binary | ios::in | ios::out);
    }

    if (!file_.is_open())
    {
        throw runtime_error("failed to open wal files");
    }
    recover();
}

bool wal::WAL::append(const string &payload)
{
    // go to eof
    file_.seekp(0, ios::end);

    // here we need to write them using there bytes and not raw value and hence we get their addresses but to write as bytes we must force convert to that data type first
    uint32_t crc_value = calculate_crc(reinterpret_cast<const uint8_t *>(payload.data()), payload.size());

    RecordHeader header;

    header.lsn = next_lsn_;
    header.payload_size = payload.size();
    header.crc32 = crc_value;

    // we write header data
    file_.write(reinterpret_cast<const char *>(&header), sizeof(header));
    // we write payload data
    file_.write((payload.data()), payload.size());

    if (!file_)
        return false;

    next_lsn_++;

    return true;
}

#ifndef WAL_NO_MAIN
int main()
{
    wal::WAL wal("test.wal");

    wal.append("hello arnav");
    wal.append("hello man");
    wal.flush();

    return 0;
}
#endif
