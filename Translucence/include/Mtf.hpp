#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>

namespace Translucence {

// A lightweight handle to a pooled string
typedef uint32_t StringId;
constexpr StringId InvalidStringId = 0xFFFFFFFF;

// Handles zero-allocation string storage
class StringPool {
private:
    std::vector<std::string> buffer;
    std::unordered_map<std::string_view, StringId> lookup;

public:
    StringId intern(std::string_view text);
    const std::string& resolve(StringId id) const;
};

// Represents a parsed block: [CategoryName]
struct MTFCategory {
    StringId nameId = InvalidStringId;
    
    std::vector<StringId> notes;
    std::vector<StringId> listEntries; // For 1D lists (- item)
    std::unordered_map<StringId, StringId> dictEntries; // For 2D dicts (Key -> Val)
    
    std::vector<StringId> schema; // For 3D grids (@schema)
    std::unordered_map<StringId, std::vector<StringId>> gridEntries; // Key -> [Val1, Val2]
};

class MTFDatabase {
private:
    StringPool stringPool;
    std::unordered_map<StringId, MTFCategory> categories;

    // Parsing helpers
    std::string_view trim(std::string_view str);
    std::vector<std::string_view> tokenizeArray(std::string_view content);

public:
    MTFDatabase() = default;

    // Core Loader
    bool loadFromFile(const std::string& filepath);

    // Frontend Engine API (Returns actual strings for your UI/Gameplay)
    // Check if a category exists
    bool hasCategory(const std::string& categoryName);

    // Get all notes attached to a category
    std::vector<std::string> getNotes(const std::string& categoryName);

    // Get a 1D List
    std::vector<std::string> getList(const std::string& categoryName);

    // Get a 2D Dict Value
    std::string getDictValue(const std::string& categoryName, const std::string& key);

    // Get a 3D Grid Value
    std::string getGridValue(const std::string& categoryName, const std::string& rowKey, const std::string& schemaKey);
};

} // namespace Translucence