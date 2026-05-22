//
// Created by Štěpán Toman on 21.05.2026.
//

#include "../include/Mtf.hpp"
#include <algorithm>

MTF::MTF(const std::string& filePath) : file(filePath), currentCategory("") {
}

std::string_view MTF::trim(std::string_view str) {
    const size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return "";
    const size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void MTF::startCategory(const std::string& name) {
    std::string cleanName = std::string(trim(name));
    if (cleanName.empty()) return;

    currentCategory = cleanName;

    auto it = std::find_if(orderedData.begin(), orderedData.end(), [&](const CategoryNode& node) {
        return node.name == currentCategory;
    });

    if (it == orderedData.end()) {
        orderedData.push_back({currentCategory, {}});
    }
}

void MTF::endCategory() {
    currentCategory = "";
}

void MTF::addEntry(const std::string& keyword, const std::string& value) {
    if (currentCategory.empty() || keyword.empty()) return;

    fastLookupMap[currentCategory][keyword] = value;

    auto it = std::find_if(orderedData.begin(), orderedData.end(), [&](const CategoryNode& node) {
        return node.name == currentCategory;
    });

    if (it != orderedData.end()) {
        auto entryIt = std::find_if(it->entries.begin(), it->entries.end(), [&](const Entry& e) {
            return e.key == keyword;
        });

        if (entryIt != it->entries.end()) {
            entryIt->value = value;
        } else {
            it->entries.push_back({keyword, value});
        }
    }
}

bool MTF::save() {
    std::string output = "#MTF\n";

    for (const auto& category : orderedData) {
        output += category.name + "{\n";
        for (const auto& entry : category.entries) {
            output += "\t[" + entry.value + "] -> " + entry.key + "\n";
        }
        output += "<-}\n";
    }

    return file.write(output);
}

bool MTF::load() {
    clear();
    if (!file.exists()) return false;

    std::string rawContent = file.read();
    std::string_view stream(rawContent);

    size_t pos = 0;
    std::string activeCat = "";

    while (pos < stream.size()) {
        size_t nextLine = stream.find('\n', pos);
        if (nextLine == std::string_view::npos) nextLine = stream.size();

        std::string_view line = trim(stream.substr(pos, nextLine - pos));
        pos = nextLine + 1;

        if (line.empty() || line[0] == '#' || line.rfind("//", 0) == 0) {
            continue;
        }

        if (line == "<-}") {
            activeCat = "";
            continue;
        }

        if (line.back() == '{') {
            activeCat = std::string(trim(line.substr(0, line.size() - 1)));
            startCategory(activeCat);
            continue;
        }

        if (!activeCat.empty() && line[0] == '[') {
            const size_t closeBracket = line.find(']');
            const size_t arrow = line.find("->");

            if (closeBracket != std::string_view::npos && arrow != std::string_view::npos && arrow > closeBracket) {
                std::string val = std::string(line.substr(1, closeBracket - 1));
                std::string key = std::string(trim(line.substr(arrow + 2)));

                if (!key.empty()) {
                    currentCategory = activeCat;
                    addEntry(key, val);
                }
            }
        }
    }

    currentCategory = "";
    return true;
}

void MTF::clear() {
    orderedData.clear();
    fastLookupMap.clear();
    currentCategory = "";
}

std::string MTF::getValue(const std::string& category, const std::string& keyword) const {
    auto catIt = fastLookupMap.find(category);
    if (catIt != fastLookupMap.end()) {
        auto keyIt = catIt->second.find(keyword);
        if (keyIt != catIt->second.end()) {
            return keyIt->second;
        }
    }
    return "";
}

const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& MTF::getData() const {
    return fastLookupMap;
}