//
// Created by Stěpán Toman on 20.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_MWFILE_HPP
#define TRANSLUCENCEWORKSPACE_MWFILE_HPP

#include <T_Core.hpp>
#include <File.hpp>
#include <vector>
#include <array>
#include <string>

class MwFile {
public:
    explicit MwFile(const std::string& path);
    ~MwFile();

    [[nodiscard]] uintmax_t size() const {return file.size();}
    bool verify();
    void writeAtByte(int byte, const std::vector<Uint8>& data);
    void writeASCII(int byte, const std::string& text);
    void writeAddress(int byte, int data);
    std::vector<Uint8> readBytes(int byte, int bytesAmount);
    void clear();
    File* getFileClass();

    template <typename T>
    void writeArray(int byte, T* arr, size_t size) {
        writeAtByte(byte, {0x5B, 0x2F});
        for(size_t i = 0; i < size; ++i) {
            const Uint8* ptr = reinterpret_cast<const Uint8*>(&arr[i]);
            writeAtByte(byte + 2 + i * sizeof(T), std::vector<Uint8>(ptr, ptr + sizeof(T)));
        }
        writeAtByte(byte + 2 + size * sizeof(T), {0x5C, 0x5D});
    }


private:
    File file;
    std::string path;
    static constexpr std::array<Uint8, 4> MAGIC = {0x4D, 0x57, 0xE5, 0x3E};
};
#endif //TRANSLUCENCEWORKSPACE_MWFILE_HPP