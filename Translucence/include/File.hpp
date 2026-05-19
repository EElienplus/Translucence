#ifndef TRANSLUCENCEWORKSPACE_FILE_HPP
#define TRANSLUCENCEWORKSPACE_FILE_HPP

#include <fstream>

#include "T_Core.hpp"

namespace Translucence {
    class File {
    public:
        static constexpr std::string_view EXTENSION = ".tc";

        explicit File(fs::path path)
            : filePath(std::move(path)),
              valid(!filePath.empty()) {
            if (filePath.extension() != EXTENSION) {
                filePath.replace_extension(EXTENSION);
            }
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
            if (!exists()) {
                return {};
            }

            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            if (!file) {
                return {};
            }

            const auto fileSize = size();
            std::vector<uint8_t> buffer(fileSize);

            file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
            return buffer;
        }

        bool writeBytes(const std::vector<uint8_t>& data) const {
            std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file) {
                return false;
            }

            file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
            return file.good();
        }

    private:
        bool writeToFile(const std::string& content, std::ios::openmode mode) const {
            std::ofstream file(filePath, std::ios::out | std::ios::binary | mode);
            if (!file) {
                return false;
            }

            file.write(content.data(), static_cast<std::streamsize>(content.size()));
            return file.good();
        }

        fs::path filePath;
        bool valid = false;
    };
} // namespace Translucence

#endif // TRANSLUCENCEWORKSPACE_FILE_HPP
