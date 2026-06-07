#pragma once

#include <string>
#include<fstream>
#include <cstdint>

namespace wal
{

#pragma pack(push, 1)

    struct RecordHeader
    {
        uint64_t lsn;
        uint32_t payload_size;
        uint32_t crc32;
    };
#pragma pack(pop)

    class WAL
    {
    public:
        explicit WAL(const std::string &file_path);
        ~WAL();

        bool append(const std::string &payload);
        bool flush();
        bool recover();
    
        private:
        std::fstream file_;
        std::string file_path_;
        uint64_t next_lsn_=1;
    
    
    };



}