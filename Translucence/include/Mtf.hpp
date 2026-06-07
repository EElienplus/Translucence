#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <string_view>

namespace Translucence {

class MTFNode {
public:
    std::string name;
    
    // Properties (key -> value)
    std::map<std::string, std::string> properties;
    
    // Lists (- item)
    std::vector<std::string> list;
    
    // Grids (key -> [v1, v2, ...])
    std::map<std::string, std::vector<std::string>> grids;
    
    // Nested sections [Section/Subsection]
    std::map<std::string, std::shared_ptr<MTFNode>> children;

    MTFNode(const std::string& n = "") : name(n) {}

    // --- High-level API ---

    // Property access
    void set(const std::string& key, const std::string& val) { properties[key] = val; }
    std::string get(const std::string& key, const std::string& def = "") const {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : def;
    }

    // List access
    void add(const std::string& item) { list.push_back(item); }
    const std::vector<std::string>& getList() const { return list; }

    // Grid access
    void setGrid(const std::string& key, const std::vector<std::string>& vals) { grids[key] = vals; }
    std::vector<std::string> getGrid(const std::string& key) const {
        auto it = grids.find(key);
        return (it != grids.end()) ? it->second : std::vector<std::string>();
    }

    // Path-based child access (e.g., "Sub/Section")
    std::shared_ptr<MTFNode> find(std::string_view path, bool create = false);
    
    // Check if node has any data
    bool empty() const {
        return properties.empty() && list.empty() && grids.empty() && children.empty();
    }
};

/**
 * A simple but robust hierarchical data manager.
 * Acts like a virtual filesystem for structured game data.
 */
class MTF {
public:
    MTF();

    // Core IO
    bool load(const std::string& filepath);
    bool loadFromString(const std::string& content);
    bool save(const std::string& filepath);
    std::string saveToString();

    // Accessors
    std::shared_ptr<MTFNode> root() { return rootNode; }
    
    // Path-based access: mtf.section("Protocols/ALPHA")
    std::shared_ptr<MTFNode> section(const std::string& path, bool create = true) {
        return rootNode->find(path, create);
    }

    // Serialization shortcuts
    void clear();

private:
    std::shared_ptr<MTFNode> rootNode;

    // Parsing helpers
    std::string_view trim(std::string_view s);
    std::vector<std::string> tokenizeArray(std::string_view content);
};

} // namespace Translucence
