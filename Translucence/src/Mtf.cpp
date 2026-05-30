#include "MTF.hpp"
#include <fstream>
#include <iostream>

namespace Translucence {

StringId StringPool::intern(std::string_view text) {
    if (text.empty()) return InvalidStringId;

    auto it = lookup.find(text);
    if (it != lookup.end()) {
        return it->second;
    }
    
    buffer.emplace_back(text);
    StringId newId = static_cast<StringId>(buffer.size() - 1);
    
    // Update lookup with a stable string_view pointing to the newly stored string
    lookup[std::string_view(buffer.back())] = newId;
    return newId;
}

const std::string& StringPool::resolve(StringId id) const {
    static const std::string empty = "";
    if (id == InvalidStringId || id >= buffer.size()) return empty;
    return buffer[id];
}

std::string_view MTFDatabase::trim(std::string_view str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string_view> MTFDatabase::tokenizeArray(std::string_view content) {
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = content.find(',');
    
    while (end != std::string_view::npos) {
        tokens.push_back(trim(content.substr(start, end - start)));
        start = end + 1;
        end = content.find(',', start);
    }
    tokens.push_back(trim(content.substr(start)));
    return tokens;
}

bool MTFDatabase::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[MTF] Failed to open: " << filepath << "\n";
        return false;
    }

    std::string line;
    MTFCategory* currentCat = nullptr;

    while (std::getline(file, line)) {
        std::string_view view = trim(line);
        if (view.empty()) continue;

        // 1. Detect Category Block
        if (view.front() == '[' && view.back() == ']') {
            std::string_view catName = trim(view.substr(1, view.length() - 2));
            StringId catId = stringPool.intern(catName);
            categories[catId] = MTFCategory();
            currentCat = &categories[catId];
            currentCat->nameId = catId;
            continue;
        }

        if (!currentCat) continue; // Skip orphan data

        if (view.front() == '-') {
            std::string_view item = trim(view.substr(1));
            currentCat->listEntries.push_back(stringPool.intern(item));
            continue;
        }

        size_t arrowPos = view.find("->");
        if (arrowPos == std::string_view::npos) continue;

        std::string_view keyStr = trim(view.substr(0, arrowPos));
        std::string_view valStr = trim(view.substr(arrowPos + 2));

        if (keyStr == "@note") {
            if (valStr.front() == '"' && valStr.back() == '"') {
                valStr = valStr.substr(1, valStr.length() - 2);
            }
            currentCat->notes.push_back(stringPool.intern(valStr));
            continue;
        }

        if (keyStr == "@schema") {
            if (valStr.front() == '[' && valStr.back() == ']') {
                std::string_view content = valStr.substr(1, valStr.length() - 2);
                auto tokens = tokenizeArray(content);
                for (auto token : tokens) {
                    currentCat->schema.push_back(stringPool.intern(token));
                }
            }
            continue;
        }

        if (valStr.front() == '[' && valStr.back() == ']') {
            std::string_view content = valStr.substr(1, valStr.length() - 2);
            auto tokens = tokenizeArray(content);
            
            StringId rowKeyId = stringPool.intern(keyStr);
            std::vector<StringId>& rowData = currentCat->gridEntries[rowKeyId];
            
            for (auto token : tokens) {
                rowData.push_back(stringPool.intern(token));
            }
            continue;
        }

        // 6. Fallback to 2D Dict
        currentCat->dictEntries[stringPool.intern(keyStr)] = stringPool.intern(valStr);
    }

    return true;
}

bool MTFDatabase::hasCategory(const std::string& categoryName) {
    StringId catId = stringPool.intern(categoryName);
    return categories.find(catId) != categories.end();
}

std::vector<std::string> MTFDatabase::getNotes(const std::string& categoryName) {
    std::vector<std::string> result;
    StringId catId = stringPool.intern(categoryName);
    if (categories.find(catId) == categories.end()) return result;
    
    for (StringId noteId : categories[catId].notes) {
        result.push_back(stringPool.resolve(noteId));
    }
    return result;
}

std::vector<std::string> MTFDatabase::getList(const std::string& categoryName) {
    std::vector<std::string> result;
    StringId catId = stringPool.intern(categoryName);
    if (categories.find(catId) == categories.end()) return result;
    
    for (StringId itemId : categories[catId].listEntries) {
        result.push_back(stringPool.resolve(itemId));
    }
    return result;
}

std::string MTFDatabase::getDictValue(const std::string& categoryName, const std::string& key) {
    StringId catId = stringPool.intern(categoryName);
    StringId keyId = stringPool.intern(key);
    
    if (categories.find(catId) != categories.end()) {
        auto& dict = categories[catId].dictEntries;
        if (dict.find(keyId) != dict.end()) {
            return stringPool.resolve(dict[keyId]);
        }
    }
    return "";
}

std::string MTFDatabase::getGridValue(const std::string& categoryName, const std::string& rowKey, const std::string& schemaKey) {
    StringId catId = stringPool.intern(categoryName);
    StringId rowId = stringPool.intern(rowKey);
    StringId schemaId = stringPool.intern(schemaKey);

    if (categories.find(catId) == categories.end()) return "";
    
    MTFCategory& cat = categories[catId];
    if (cat.gridEntries.find(rowId) == cat.gridEntries.end()) return "";

    // Find which column index the requested schema key belongs to
    int targetColIndex = -1;
    for (size_t i = 0; i < cat.schema.size(); ++i) {
        if (cat.schema[i] == schemaId) {
            targetColIndex = static_cast<int>(i);
            break;
        }
    }

    if (targetColIndex == -1) return ""; // Schema column doesn't exist

    std::vector<StringId>& rowData = cat.gridEntries[rowId];
    if (targetColIndex < rowData.size()) {
        return stringPool.resolve(rowData[targetColIndex]);
    }

    return "";
}

} // namespace Translucence