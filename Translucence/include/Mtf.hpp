//
// Created by Štěpán Toman on 21.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_MTF_HPP
#define TRANSLUCENCEWORKSPACE_MTF_HPP

#include <File.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>

class MTF {
public:
    explicit MTF(const std::string& filePath);
    ~MTF() = default;

    void startCategory(const std::string& name);
    void endCategory();
    void addEntry(const std::string& keyword, const std::string& value);

    bool save();
    bool load();
    void clear();

    [[nodiscard]] std::string getValue(const std::string& category, const std::string& keyword) const;
    [[nodiscard]] const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& getData() const;

private:
    File file;

    struct Entry {
        std::string key;
        std::string value;
    };

    struct CategoryNode {
        std::string name;
        std::vector<Entry> entries;
    };

    std::vector<CategoryNode> orderedData;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> fastLookupMap;

    std::string currentCategory;

    static std::string_view trim(std::string_view str);
};

#endif //TRANSLUCENCEWORKSPACE_MTF_HPP