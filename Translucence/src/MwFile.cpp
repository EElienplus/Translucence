//
// Created by Stěpán Toman on 20.05.2026.
//

#include <MwFile.hpp>

MwFile::MwFile(const std::string& path) : file(path), path(path) {
    file.create();
    file.writeBytesAt(0, {MAGIC.begin(), MAGIC.end()});
}

MwFile::~MwFile() = default;

File* MwFile::getFileClass() {
    return &file;
}

bool MwFile::verify() {
    return file.readBytesAt(0, 4) == std::vector<Uint8>{MAGIC.begin(), MAGIC.end()};
}

void MwFile::writeAtByte(int byte, const std::vector<Uint8>& data) {
    file.writeBytesAt(byte, data);
}

void MwFile::writeASCII(int byte, const std::string& text) {
    std::vector<Uint8> data(text.begin(), text.end());

    writeAtByte(byte, {0xE5});
    writeAtByte(byte + 1, data);
    writeAtByte(byte + 1 + data.size(), {0x5C, 0xC5});
}

void MwFile::writeAddress(int byte, int data) {
    writeAtByte(byte, {0x24});
    writeAtByte(byte + 1, {static_cast<unsigned char>(data)});
}

std::vector<Uint8> MwFile::readBytes(int byte, int bytesAmount) {
    return file.readBytesAt(byte, bytesAmount);
}

void MwFile::clear() {
    file.clear();
    file.writeBytesAt(0, {MAGIC.begin(), MAGIC.end()});
}