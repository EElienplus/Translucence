#ifndef TRANSLUCENCEWORKSPACE_FILE_HPP
#define TRANSLUCENCEWORKSPACE_FILE_HPP

#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <string_view>
#include "T_Core.hpp"

namespace Translucence {

    class File {
    public:
        static constexpr std::string_view EXTENSION = ".tc";

        explicit File(std::filesystem::path p) : m_path(std::move(p)) {
            if (m_path.extension() != EXTENSION) {
                m_path.replace_extension(EXTENSION);
            }
            m_valid = !m_path.empty();
        }


        [[nodiscard]] bool exists() const {
            return std::filesystem::exists(m_path);
        }

        [[nodiscard]] bool isValid() const {
            return m_valid;
        }

        [[nodiscard]] uintmax_t size() const {
            return exists() ? std::filesystem::file_size(m_path) : 0;
        }

        [[nodiscard]] const std::filesystem::path& path() const {
            return m_path;
        }

        // --- Lifecycle ---

        bool create() const {
            if (exists()) return false;
            if (m_path.has_parent_path()) {
                std::filesystem::create_directories(m_path.parent_path());
            }
            std::ofstream file(m_path, std::ios::out | std::ios::binary);
            return file.good();
        }

        bool remove() const {
            return std::filesystem::remove(m_path);
        }


        [[nodiscard]] std::string read() const {
            if (!exists()) return "";
            std::ifstream file(m_path, std::ios::in | std::ios::binary);
            const auto fileSize = size();
            std::string buffer(fileSize, '\0');
            file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
            return buffer;
        }

        bool write(const std::string& content) const {
            std::ofstream file(m_path, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file) return false;
            file.write(content.data(), content.size());
            return file.good();
        }

        bool append(const std::string& content) const {
            std::ofstream file(m_path, std::ios::out | std::ios::binary | std::ios::app);
            if (!file) return false;
            file.write(content.data(), content.size());
            return file.good();
        }

        void addLine() const {
            append("\n");
        }

        // Just some stuff I'm testing. dw about it

        [[nodiscard]] std::vector<uint8_t> readBytes() const {
            if (!exists()) return {};
            std::ifstream file(m_path, std::ios::in | std::ios::binary);
            const auto fileSize = size();
            std::vector<uint8_t> buffer(fileSize);
            file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
            return buffer;
        }

        bool writeBytes(const std::vector<uint8_t>& data) const {
            std::ofstream file(m_path, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file) return false;
            file.write(reinterpret_cast<const char*>(data.data()), data.size());
            return file.good();
        }

    private:
        std::filesystem::path m_path;
        bool m_valid = false;
    };

} // namespace Translucence

#endif // TRANSLUCENCEWORKSPACE_FILE_HPP