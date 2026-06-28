#pragma once

#include <string>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include "wal.h"

namespace fault
{

// byte offset where record `index` (0-based) starts
inline std::streamoff record_offset(const std::string &path, std::size_t index)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
        throw std::runtime_error("corrupt: cannot open " + path);

    std::streamoff offset = 0;
    for (std::size_t i = 0; i < index; ++i)
    {
        wal::RecordHeader h;
        f.read(reinterpret_cast<char *>(&h), sizeof(h));
        if (!f)
            throw std::runtime_error("corrupt: record index out of range");

        offset += static_cast<std::streamoff>(sizeof(h)) + h.payload_size;
        f.seekg(offset, std::ios::beg);
    }
    return offset;
}

// flip a bit in record `index`'s payload so it no longer matches its crc
inline void flip_payload_bit(const std::string &path, std::size_t index)
{
    const std::streamoff off =
        record_offset(path, index) + static_cast<std::streamoff>(sizeof(wal::RecordHeader));

    std::fstream f(path, std::ios::in | std::ios::out | std::ios::binary);
    f.seekg(off);
    char b;
    f.read(&b, 1);
    b ^= 0x01;
    f.seekp(off);
    f.write(&b, 1);
}

// overwrite record `index`'s payload_size with a bogus value
inline void smash_payload_size(const std::string &path, std::size_t index, uint32_t bogus)
{
    const std::streamoff off =
        record_offset(path, index) +
        static_cast<std::streamoff>(offsetof(wal::RecordHeader, payload_size));

    std::fstream f(path, std::ios::in | std::ios::out | std::ios::binary);
    f.seekp(off);
    f.write(reinterpret_cast<const char *>(&bogus), sizeof(bogus));
}

// keep records 0..keep-1 whole, leave `extra` dangling bytes of the next one
inline void truncate_to(const std::string &path, std::size_t keep, std::streamoff extra = 0)
{
    const std::streamoff off = record_offset(path, keep) + extra;
    std::filesystem::resize_file(path, static_cast<std::uintmax_t>(off));
}

} // namespace fault
