#ifndef TRANSLUCENCEWORKSPACE_FILE_HPP
#define TRANSLUCENCEWORKSPACE_FILE_HPP

#include <fstream>
#include <vector>
#include <filesystem>

#ifndef TRANSLUCENCEWORKSPACE_CORE_HPP
namespace fs = std::filesystem;
#endif

    class File {
    public:
        explicit File(fs::path path)
            : filePath(std::move(path)),
              valid(!filePath.empty()) {
        }

        [[nodiscard]] bool exists() const {
            return fs::exists(filePath);
        }

        [[nodiscard]] bool isValid() const {
            return valid;
        }

        [[nodiscard]] uintmax_t size() const {
            return exists() ? fs::file_size(filePath) : 0;
        }

        [[nodiscard]] const fs::path& path() const {
            return filePath;
        }

        bool create() const {
            if (exists()) {
                return false;
            }

            if (filePath.has_parent_path()) {
                fs::create_directories(filePath.parent_path());
            }

            std::ofstream file(filePath, std::ios::out | std::ios::binary);
            return file.good();
        }

        bool remove() const {
            return fs::remove(filePath);
        }

        [[nodiscard]] std::string read() const {
            if (!exists()) {
                return "";
            }

            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            if (!file) {
                return "";
            }

            const auto fileSize = size();
            std::string buffer(fileSize, '\0');

            file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
            return buffer;
        }

        bool write(const std::string& content) const {
            return writeToFile(content, std::ios::trunc);
        }

        bool append(const std::string& content) const {
            return writeToFile(content, std::ios::app);
        }

        bool addLine() const {
            return append("\n");
        }

        [[nodiscard]] std::vector<uint8_t> readBytes() const {
            return readBytesAt(0, static_cast<size_t>(size()));
        }

        [[nodiscard]] std::vector<uint8_t> readBytesAt(size_t offset, size_t bytesAmount) const {
            if (!exists()) {
                return {};
            }

            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            if (!file) {
                return {};
            }

            file.seekg(static_cast<std::streamoff>(offset));
            
            const auto fileSize = size();
            if (offset >= fileSize) {
                return {};
            }

            const size_t actualAmount = std::min(bytesAmount, static_cast<size_t>(fileSize - offset));
            std::vector<uint8_t> buffer(actualAmount);

            file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(actualAmount));
            return buffer;
        }

        bool writeBytes(const std::vector<uint8_t>& data) const {
            return writeToFile(reinterpret_cast<const char*>(data.data()), data.size(), std::ios::trunc);
        }

        bool writeBytesAt(size_t offset, const std::vector<uint8_t>& data) const {
            if (data.empty()) {
                return true;
            }
            
            // If file doesn't exist, we just write the data at the beginning regardless of offset (or should we pad?)
            // Usually writeBytesAt implies we can write anywhere.
            // If offset > size, we might want to pad with zeros, but let's follow standard behavior.
            std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
            
            if (!file) {
                // If file doesn't exist or couldn't be opened in in/out mode, try creating it
                if (!exists()) {
                    create();
                    file.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
                }
                
                if (!file) {
                    return false;
                }
            }

            file.seekp(static_cast<std::streamoff>(offset));
            file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
            return file.good();
        }

        bool clear() const {
            std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
            return file.good();
        }

        bool rename(const fs::path& newPath) {
            try {
                fs::rename(filePath, newPath);
                filePath = newPath;
                return true;
            } catch (...) {
                return false;
            }
        }

    private:
        bool writeToFile(const std::string& content, std::ios::openmode mode) const {
            return writeToFile(content.data(), content.size(), mode);
        }

        bool writeToFile(const char* data, size_t size, std::ios::openmode mode) const {
            std::ofstream file(filePath, std::ios::out | std::ios::binary | mode);
            if (!file) {
                return false;
            }

            file.write(data, static_cast<std::streamsize>(size));
            return file.good();
        }

        fs::path filePath;
        bool valid = false;
    };

#endif // TRANSLUCENCEWORKSPACE_FILE_HPP
