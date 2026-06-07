#include "Mtf.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace Translucence {

std::shared_ptr<MTFNode> MTFNode::find(std::string_view path, bool create) {
    if (path.empty()) return nullptr;

    size_t slash = path.find('/');
    std::string_view first = path;
    std::string_view rest = "";

    if (slash != std::string_view::npos) {
        first = path.substr(0, slash);
        rest = path.substr(slash + 1);
    }

    std::string nameStr(first);
    if (!children.count(nameStr)) {
        if (create) {
            children[nameStr] = std::make_shared<MTFNode>(nameStr);
        } else {
            return nullptr;
        }
    }

    if (rest.empty()) {
        return children[nameStr];
    } else {
        return children[nameStr]->find(rest, create);
    }
}

MTF::MTF() {
    rootNode = std::make_shared<MTFNode>("root");
}

void MTF::clear() {
    rootNode = std::make_shared<MTFNode>("root");
}

std::string_view MTF::trim(std::string_view s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

std::vector<std::string> MTF::tokenizeArray(std::string_view content) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = content.find(',');
    
    while (end != std::string_view::npos) {
        tokens.emplace_back(trim(content.substr(start, end - start)));
        start = end + 1;
        end = content.find(',', start);
    }
    tokens.emplace_back(trim(content.substr(start)));
    return tokens;
}

bool MTF::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    clear();
    std::string line;
    std::shared_ptr<MTFNode> currentSection = rootNode;

    while (std::getline(file, line)) {
        std::string_view view = trim(line);
        if (view.empty() || view.front() == '#') continue;

        // 1. Section: [Path/To/Section]
        if (view.front() == '[' && view.back() == ']') {
            std::string_view path = trim(view.substr(1, view.length() - 2));
            currentSection = section(std::string(path), true);
            continue;
        }

        // 2. List Item: - item
        if (view.front() == '-') {
            currentSection->add(std::string(trim(view.substr(1))));
            continue;
        }

        // 3. Key-Value: key -> value
        size_t arrow = view.find("->");
        if (arrow != std::string_view::npos) {
            std::string_view key = trim(view.substr(0, arrow));
            std::string_view val = trim(view.substr(arrow + 2));

            // Metadata: @schema
            if (key == "@schema" || (val.front() == '[' && val.back() == ']')) {
                std::string_view content = val;
                if (val.front() == '[' && val.back() == ']') {
                    content = val.substr(1, val.length() - 2);
                }
                currentSection->setGrid(std::string(key), tokenizeArray(content));
            } else {
                // Strip quotes if present
                if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                    val = val.substr(1, val.length() - 2);
                }
                currentSection->set(std::string(key), std::string(val));
            }
            continue;
        }
    }

    return true;
}

bool MTF::loadFromString(const std::string& content) {
    std::stringstream ss(content);
    clear();
    std::string line;
    std::shared_ptr<MTFNode> currentSection = rootNode;

    while (std::getline(ss, line)) {
        std::string_view view = trim(line);
        if (view.empty() || view.front() == '#') continue;

        if (view.front() == '[' && view.back() == ']') {
            std::string_view path = trim(view.substr(1, view.length() - 2));
            currentSection = section(std::string(path), true);
            continue;
        }

        if (view.front() == '-') {
            currentSection->add(std::string(trim(view.substr(1))));
            continue;
        }

        size_t arrow = view.find("->");
        if (arrow != std::string_view::npos) {
            std::string_view key = trim(view.substr(0, arrow));
            std::string_view val = trim(view.substr(arrow + 2));

            if (key == "@schema" || (val.front() == '[' && val.back() == ']')) {
                std::string_view arrContent = val;
                if (val.front() == '[' && val.back() == ']') {
                    arrContent = val.substr(1, val.length() - 2);
                }
                currentSection->setGrid(std::string(key), tokenizeArray(arrContent));
            } else {
                if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                    val = val.substr(1, val.length() - 2);
                }
                currentSection->set(std::string(key), std::string(val));
            }
            continue;
        }
    }
    return true;
}

static void saveNodeRecursive(std::ostream& os, std::shared_ptr<MTFNode> node, const std::string& fullPath) {
    if (!node->empty() && node->name != "root") {
        os << "[" << fullPath << "]\n";
        
        // Properties
        for (const auto& [key, val] : node->properties) {
            os << key << " -> \"" << val << "\"\n";
        }
        
        // Grids
        for (const auto& [key, vals] : node->grids) {
            os << key << " -> [";
            for (size_t i = 0; i < vals.size(); ++i) {
                os << vals[i] << (i < vals.size() - 1 ? ", " : "");
            }
            os << "]\n";
        }
        
        // List
        for (const auto& item : node->list) {
            os << "- " << item << "\n";
        }
        
        os << "\n";
    }

    // Children
    for (const auto& [name, child] : node->children) {
        std::string nextPath = fullPath.empty() ? name : fullPath + "/" + name;
        saveNodeRecursive(os, child, nextPath);
    }
}

bool MTF::save(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    saveNodeRecursive(file, rootNode, "");
    return true;
}

std::string MTF::saveToString() {
    std::stringstream ss;
    saveNodeRecursive(ss, rootNode, "");
    return ss.str();
}

} // namespace Translucence
