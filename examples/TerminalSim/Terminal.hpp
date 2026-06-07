#include <Translucence.hpp>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <queue>

namespace fs = std::filesystem;

class Terminal {
public:
    struct CommandContext {
        std::string command;
        std::vector<std::string> args;
        Terminal& terminal;
        
        std::string getFullArgs() const {
            std::string result;
            for (size_t i = 0; i < args.size(); ++i) {
                result += args[i];
                if (i < args.size() - 1) result += " ";
            }
            return result;
        }

        std::string getArg(size_t index, const std::string& defaultValue = "") const {
            if (index < args.size()) return args[index];
            return defaultValue;
        }

        bool hasArgs(size_t count) const { return args.size() >= count; }
        
        bool checkArgs(size_t required, const std::string& usageMsg = "") {
            if (args.size() < required) {
                terminal.println(usageMsg.empty() ? "Error: Missing arguments for '" + command + "'" : "Usage: " + usageMsg);
                return false;
            }
            return true;
        }

        bool is(const std::string& op) const {
            return !args.empty() && args[0] == op;
        }

        bool checkOpArgs(const std::string& op, size_t required, const std::string& usageMsg) {
            if (is(op)) {
                if (args.size() < required + 1) {
                    terminal.println("Usage: " + usageMsg);
                    return false;
                }
                return true;
            }
            return false;
        }

        std::string getOpArg(const std::string& op, size_t index) const {
            if (is(op) && args.size() > index + 1) return args[index + 1];
            return "";
        }

        void error(const std::string& msg) { terminal.println("Error: " + msg); }
        void println(const std::string& msg) { terminal.println(msg); }
        int getClearance() const { return terminal.getClearance(); }
    };

    using CommandHandler = std::function<void(CommandContext&)>;

    struct CommandInfo {
        CommandHandler handler;
        std::string description;
        int minClearance = 0;
    };

    Terminal() {
        // Default to current directory if not set later
        rootDir = fs::current_path();
        currentDir = rootDir;

        field.enabled = true;
        field.multiLine = true;
        field.onTextSubmit = [this](const std::string& input) {
            handleInput(input);
        };
    }

    void registerBuiltinCommands() {
        addCommand("help", [this](CommandContext& /*ctx*/) {
            println("Available commands:");
            for (const auto& [name, info] : commands) {
                std::string line = "  " + name;
                if (!info.description.empty()) {
                    line += " - " + info.description;
                }
                println(line);
            }
        }, "Shows this help message");

        addCommand("clear", [](CommandContext& ctx) {
            ctx.terminal.clear();
        }, "Clears the terminal screen");

        addCommand("ls", [this](CommandContext& ctx) {
            try {
                std::string arg = ctx.getArg(0);
                fs::path targetDir = arg.empty() ? currentDir : resolvePath(arg);
                
                if (!isWithinRoot(targetDir)) {
                    ctx.terminal.println("ls: " + (arg.empty() ? "." : arg) + ": No such file or directory");
                    return;
                }

                if (!fs::exists(targetDir)) {
                    ctx.terminal.println("ls: " + (arg.empty() ? "." : arg) + ": No such file or directory");
                    return;
                }

                if (!fs::is_directory(targetDir)) {
                    ctx.terminal.println(targetDir.filename().string());
                    return;
                }

                std::vector<fs::path> entries;
                for (const auto& entry : fs::directory_iterator(targetDir)) {
                    entries.push_back(entry.path());
                }
                std::sort(entries.begin(), entries.end());
                
                for (const auto& path : entries) {
                    std::string name = path.filename().string();
                    if (name.front() == '.' && name != "." && name != "..") continue;

                    if (fs::is_directory(path)) {
                        ctx.terminal.println(name + "/");
                    } else {
                        ctx.terminal.println(name);
                    }
                }
            } catch (const std::exception& e) {
                ctx.terminal.println("ls: error: " + std::string(e.what()));
            }
        }, "List files in the current directory");

        addCommand("cd", [this](CommandContext& ctx) {
            std::string target = ctx.getArg(0);
            if (target.empty()) {
                changeDirectory("/");
                return;
            }
            if (!changeDirectory(target)) {
                ctx.terminal.println("cd: " + target + ": No such directory");
            }
        }, "Change directory");

        addCommand("pwd", [this](CommandContext& ctx) {
            ctx.terminal.println(getRelativeCwd());
        }, "Print working directory");

        addCommand("cat", [this](CommandContext& ctx) {
            std::string filename = ctx.getArg(0);
            if (filename.empty()) {
                ctx.terminal.println("Usage: cat [filename]");
                return;
            }
            
            fs::path p = resolvePath(filename);
            if (!isWithinRoot(p) || !fs::exists(p)) {
                 ctx.terminal.println("cat: " + filename + ": No such file");
                 return;
            }

            if (fs::is_directory(p)) {
                ctx.terminal.println("cat: " + filename + ": Is a directory");
                return;
            }

            std::ifstream file(p);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    ctx.terminal.println(line);
                }
            } else {
                ctx.terminal.println("cat: " + filename + ": Could not open file");
            }
        }, "Read a file");
    }

    void addCommand(const std::string& name, CommandHandler handler, const std::string& description = "", int minClearance = 0) {
        commands[name] = {handler, description, minClearance};
    }

    void println(const std::string& text) {
        field.appendText(text + "\n");
    }

    void printDelayed(const std::string& text, float delaySeconds) {
        delayedActions.push({delaySeconds, [this, text]() {
            field.appendText(text);
        }});
    }

    void printDelayedLine(const std::string& text, float delaySeconds) {
        delayedActions.push({delaySeconds, [this, text]() {
            field.appendText(text + "\n");
        }});
    }

    void delayedCallback(std::function<void()> callback, float delaySeconds) {
        delayedActions.push({delaySeconds, callback});
    }

    void print(const std::string& text) {
        field.appendText(text);
    }

    void clear() {
        field.value = "";
        field.protectedLen = 0;
        field.cursorPos = 0;
    }

    void setPrompt(const std::string& p) {
        prompt = p;
    }

    void printPrompt() {
        field.appendText(prompt);
    }

    void update(float dt) {
        field.update(dt);

        if (!delayedActions.empty()) {
            delayedActions.front().remaining -= dt;

            if (delayedActions.front().remaining <= 0.0f) {
                auto action = delayedActions.front().action;
                delayedActions.pop();
                action();
            }
        }
    }

    void draw(Renderer& renderer) {
        renderer.drawInputField(field);
    }

    InputField& getField() { return field; }

    void setRect(float x, float y, float w, float h) {
        field.rect = {x, y, w, h};
    }

    void onCommandExecuted(std::function<void()> callback) {
        onCmdExecuted = callback;
    }

    void interceptNextInput(std::function<void(const std::string&)> callback) {
        inputInterceptor = callback;
    }

    void setClearance(int level) { clearance = level; }
    int getClearance() const { return clearance; }

    const std::map<std::string, CommandInfo>& getCommands() const { return commands; }

    void setRoot(const fs::path& path) {
        try {
            rootDir = fs::weakly_canonical(fs::absolute(path));
        } catch (...) {
            rootDir = fs::absolute(path).lexically_normal();
        }
        currentDir = rootDir;
    }

    fs::path getRoot() const { return rootDir; }
    fs::path getCwd() const { return currentDir; }

    std::string getRelativeCwd() const {
        if (currentDir == rootDir) return "/";
        auto rel = fs::relative(currentDir, rootDir);
        std::string s = rel.string();
        std::replace(s.begin(), s.end(), '\\', '/');
        if (s.empty() || s == ".") return "/";
        if (s[0] != '/') s = "/" + s;
        return s;
    }

    bool changeDirectory(const std::string& path) {
        fs::path newPath = resolvePath(path);
        
        if (!isWithinRoot(newPath)) {
            return false;
        }

        if (!canAccess(newPath)) {
            return false;
        }

        if (fs::exists(newPath) && fs::is_directory(newPath)) {
            currentDir = newPath;
            return true;
        }
        return false;
    }

    bool canAccess(const fs::path& path, bool silent = false) {
        try {
            fs::path absPath = fs::absolute(path).lexically_normal();
            if (absPath == rootDir) return true;

            fs::path relative = fs::relative(absPath, rootDir);
            fs::path current = rootDir;
            
            for (const auto& part : relative) {
                if (part == "." || part == "..") continue;
                current /= part;
                int req = getRequiredClearance(current);
                if (clearance < req) {
                    if (!silent) {
                        println("ACCESS DENIED: Clearance level " + std::to_string(req) + " required for '" + part.string() + "'.");
                    }
                    return false;
                }
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    int getRequiredClearance(const fs::path& path) const {
        try {
            fs::path p = fs::absolute(path).lexically_normal();
            fs::path dir = p.parent_path();
            fs::path permFile = dir / ".clearance";
            
            if (fs::exists(permFile)) {
                std::ifstream file(permFile);
                std::string line;
                std::string targetName = p.filename().string();
                while (std::getline(file, line)) {
                    if (line.empty() || line[0] == '#') continue;
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string name = line.substr(0, colon);
                        if (name == targetName || name == "*") {
                            return stringToInt(line.substr(colon + 1));
                        }
                    }
                }
            }
        } catch (...) {}
        return 0;
    }

    fs::path resolvePath(const std::string& path) const {
        if (path.empty()) return currentDir;

        fs::path p(path);
        if (p.is_absolute() || (!path.empty() && (path[0] == '/' || path[0] == '\\'))) {
            // Treat as relative to root
            std::string s = path;
            if (s[0] == '/' || s[0] == '\\') s = s.substr(1);
            return (rootDir / s).lexically_normal();
        }
        
        return (currentDir / p).lexically_normal();
    }

    bool isWithinRoot(const fs::path& path) const {
        try {
            fs::path p = fs::absolute(path).lexically_normal();
            if (p == rootDir) return true;
            
            auto rel = fs::relative(p, rootDir);
            if (rel.empty()) return false;
            
            std::string s = rel.string();
            // If relative path starts with .. then it's outside
            if (s == ".." || s.find(".." + std::string(1, fs::path::preferred_separator)) == 0) {
                return false;
            }
            
            // Also check for any absolute path return (means no common root)
            if (fs::path(s).is_absolute()) {
                return false;
            }
            
            return true;
        } catch (...) {
            return false;
        }
    }

    // High level helpers for simple command implementation
    void list(const std::string& path = "") {
        try {
            fs::path targetDir = path.empty() ? currentDir : resolvePath(path);
            if (!isWithinRoot(targetDir) || !fs::exists(targetDir)) {
                println("ls: " + (path.empty() ? "." : path) + ": No such file or directory");
                return;
            }

            if (!canAccess(targetDir)) return;

            if (!fs::is_directory(targetDir)) {
                println(targetDir.filename().string());
                return;
            }
            std::vector<fs::path> entries;
            for (const auto& entry : fs::directory_iterator(targetDir)) {
                std::string name = entry.path().filename().string();
                if (name == ".clearance") continue;
                entries.push_back(entry.path());
            }
            std::sort(entries.begin(), entries.end());
            for (const auto& p : entries) {
                println(p.filename().string() + (fs::is_directory(p) ? "/" : ""));
            }
        } catch (...) {
            println("ls: error accessing directory");
        }
    }

    void cat(const std::string& filename) {
        fs::path p = resolvePath(filename);
        if (!isWithinRoot(p) || !fs::exists(p)) {
            println("cat: " + filename + ": No such file");
            return;
        }

        if (!canAccess(p)) return;

        if (fs::is_directory(p)) {
            println("cat: " + filename + ": Is a directory");
            return;
        }
        std::ifstream file(p);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) println(line);
        } else {
            println("cat: " + filename + ": Could not open file");
        }
    }

private:
    struct DelayedPrint {
        std::string text;
        float remaining;
    };

    struct DelayedAction {
        float remaining;
        std::function<void()> action;
    };

    int clearance = 0;
    InputField field;
    std::function<void()> onCmdExecuted;
    std::function<void(const std::string&)> inputInterceptor;
    std::queue<DelayedAction> delayedActions;
    std::string prompt = "> ";
    std::map<std::string, CommandInfo> commands;
    fs::path rootDir;
    fs::path currentDir;

    void handleInput(const std::string& input) {
        // Manually append newline to echo the enter and move protection
        field.appendText("\n");
        
        std::string trimmed = input;

        if (inputInterceptor) {
            auto interceptor = inputInterceptor;
            inputInterceptor = nullptr;
            interceptor(trimmed);

            if (onCmdExecuted) onCmdExecuted();
            printPrompt();
            return;
        }
        
        if (trimmed.empty()) {
            printPrompt();
            return;
        }

        std::vector<std::string> tokens = parseLine(trimmed);
        if (tokens.empty()) {
            printPrompt();
            return;
        }

        std::string cmdName = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        if (commands.count(cmdName)) {
            const auto& info = commands[cmdName];
            if (clearance < info.minClearance) {
                println("Error: Clearance level " + std::to_string(info.minClearance) + " required for '" + cmdName + "'.");
            } else {
                CommandContext ctx = {cmdName, args, *this};
                info.handler(ctx);
            }
        } else {
            println("Unknown command: " + cmdName);
        }

        if (onCmdExecuted) onCmdExecuted();
        printPrompt();
    }

    std::vector<std::string> parseLine(const std::string& line) {
        std::vector<std::string> tokens;
        std::string current;
        bool inQuotes = false;
        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            if (c == '\"') {
                inQuotes = !inQuotes;
            } else if (std::isspace(c) && !inQuotes) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) tokens.push_back(current);
        return tokens;
    }
};
