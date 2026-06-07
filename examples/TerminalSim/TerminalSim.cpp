#include <Translucence.hpp>
#include "TerminalGame.hpp"

enum ProtocolStatus {
    RUNNING,
    IDLE,
    TERMINATED
};

struct Protocol {
    String name;
    ProtocolStatus status;
};

void askYesNo(Terminal& terminal, String question, std::function<void(int)> callback) {
    terminal.println(question + " [y/n]");

    terminal.interceptNextInput([callback](const std::string& input) {
        std::string answer = input;
        // Trim whitespace
        answer.erase(0, answer.find_first_not_of(" \t\r\n"));
        answer.erase(answer.find_last_not_of(" \t\r\n") + 1);

        std::transform(answer.begin(), answer.end(), answer.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (answer == "yes" || answer == "y" || answer == "true" || answer == "1") {
            callback(1);
            return;
        }

        if (answer == "no" || answer == "n" || answer == "false" || answer == "0") {
            callback(0);
            return;
        }

        callback(-1);
    });
}

int main() {
    Application app;
    app.create(1000, 800, "Translucence Terminal Game");
    app.setResizable(true);
    Renderer renderer(app);
    EventSystem events(app);

    Terminal terminal;
    UserManager users(terminal);

    static List<Protocol> protocols;

    // Set root relative to the executable if possible, otherwise use default
    fs::path root = "examples/TerminalSim/filesys";
    if (!fs::exists(root)) {
        fs::path search = fs::current_path();
        for (int i = 0; i < 5; ++i) {
            if (fs::exists(search / "examples/TerminalSim/filesys")) {
                root = search / "examples/TerminalSim/filesys";
                break;
            }
            if (fs::exists(search / "filesys")) {
                root = search / "filesys";
                break;
            }
            if (!search.has_parent_path()) break;
            search = search.parent_path();
        }
    }

    if (!fs::exists(root)) {
        root = fs::current_path();
    }

    terminal.setRoot(root);

    auto loadProtocols = [&]() {
        Translucence::MTF mtf;
        fs::path p = terminal.getRoot() / "protocols.mtf";
        if (mtf.load(p.string())) {
            protocols.clear();
            auto sec = mtf.section("Protocols");
            for (const auto& [name, val] : sec->properties) {
                ProtocolStatus s = IDLE;
                if (val == "RUNNING") s = RUNNING;
                else if (val == "TERMINATED") s = TERMINATED;
                protocols.push_back({name, s});
            }
        }
    };

    auto saveProtocols = [&]() {
        Translucence::MTF mtf;
        auto sec = mtf.section("Protocols");
        for (const auto& p : protocols) {
            std::string statusStr = (p.status == RUNNING ? "RUNNING" :
                                    p.status == IDLE ? "IDLE" : "TERMINATED");
            sec->set(p.name, statusStr);
        }
        mtf.save((terminal.getRoot() / "protocols.mtf").string());
    };

    loadProtocols();

    // User registrations
    users.addUser("guest", "guest", 1);
    users.addUser("admin", "super_secret_admin_password", 10);
    users.addUser("test", "test", 999);

    // Logic to update the prompt based on current state
    auto updatePrompt = [&]() {
        std::string path = terminal.getRelativeCwd();
        std::string name = users.getCurrentUser().name;
        char suffix = users.loggedIn() ? '#' : '$';
        terminal.setPrompt(name + "@translucence:" + path + suffix + " ");
    };

    updatePrompt();
    terminal.onCommandExecuted(updatePrompt);

    // --- Command Registrations ---
    terminal.addCommand("help", [&](auto& ctx) {
        ctx.terminal.println("Available commands:");
        for (const auto& [name, info] : ctx.terminal.getCommands()) {
            std::string line = "  " + name;
            if (!info.description.empty()) line += " - " + info.description;
            if (info.minClearance > 0) line += " (Clearance " + std::to_string(info.minClearance) + "+)";
            ctx.terminal.println(line);
        }
    }, "Shows this help message");
    terminal.addCommand("ls", [](auto& ctx) {
        ctx.terminal.list(ctx.getArg(0));
    }, "List files");
    terminal.addCommand("cd", [](auto& ctx) {
        std::string target = ctx.getArg(0, "/");
        if (!ctx.terminal.changeDirectory(target)) {
            ctx.error(target + ": No such directory or access denied");
        }
    }, "Change directory");
    terminal.addCommand("pwd", [](auto& ctx) {
        ctx.println(ctx.terminal.getRelativeCwd());
    }, "Print working directory");
    terminal.addCommand("cat", [](auto& ctx) {
        if (ctx.checkArgs(1, "cat [filename]")) {
            ctx.terminal.cat(ctx.getArg(0));
        }
    }, "Read a file");
    terminal.addCommand("clear", [](auto& ctx) {
        ctx.terminal.clear();
    }, "Clears the screen");
    terminal.addCommand("login", [&](auto& ctx) {
        if (users.loggedIn()) return ctx.error("Already logged in as " + users.getCurrentUser().name);

        if (!ctx.checkArgs(2, "login [user] [pass]")) return;

        std::string user = ctx.getArg(0);
        std::string pass = ctx.getArg(1);

        if (users.login(user, pass)) {
            ctx.println("Access granted. Welcome, " + user + ".");
            ctx.terminal.setClearance(users.getCurrentUser().clearance);
        } else {
            ctx.error("Access denied for user: " + user);
        }
    }, "Log in to the system");
    terminal.addCommand("logout", [&](auto& ctx) {
        if (!users.loggedIn()) return ctx.error("Not logged in.");
        users.logout();
        ctx.terminal.setClearance(0);
        ctx.println("Logged out.");
    }, "Log out of the current session");
    terminal.addCommand("whoami", [&](auto& ctx) {
        ctx.println("User: " + users.getCurrentUser().name);
        ctx.println("Clearance Level: " + std::to_string(users.getCurrentUser().clearance));
    }, "Displays current user information");
    terminal.addCommand("exit", [&](auto& /*ctx*/) {
        app.setRunning(false);
    }, "Exits the game");
    terminal.addCommand("echo", [](auto& ctx) {
        ctx.println(ctx.getFullArgs());
    }, "Echoes back the input text");
    terminal.addCommand("protocol", [&](auto& ctx) {
        if (!ctx.checkArgs(1, "protocol [list|start|terminate|delete|info|run]")) return;

        if (ctx.is("list")) {
            if (protocols.empty()) return ctx.println("No active protocols.");
            for(const Protocol& protocol : protocols) {
                std::string statusStr = (protocol.status == RUNNING ? "RUNNING" :
                                        protocol.status == IDLE ? "IDLE" : "TERMINATED");
                ctx.println("Name: " + protocol.name + ", Status: " + statusStr);
            }
        }
        else if (ctx.checkOpArgs("start", 1, "protocol start [name]")) {
            std::string name = ctx.getOpArg("start", 0);
            for (const auto& p : protocols) {
                if (p.name == name) return ctx.error("Protocol already exists: " + name);
            }
            protocols.push_back({name, ProtocolStatus::IDLE});
            saveProtocols();
            ctx.println("Initialized " + name + " protocol!");
        }
        else if (ctx.checkOpArgs("terminate", 1, "protocol terminate [name]")) {
            std::string name = ctx.getOpArg("terminate", 0);
            bool found = false;
            for (auto& p : protocols) {
                if (p.name == name) {
                    p.status = TERMINATED;
                    found = true;
                    break;
                }
            }
            if (found) {
                saveProtocols();
                ctx.println("Terminated protocol: " + name);
            } else ctx.error("Protocol not found: " + name);
        }
        else if (ctx.checkOpArgs("delete", 1, "protocol delete [name]")) {
            std::string name = ctx.getOpArg("delete", 0);
            auto it = std::find_if(protocols.begin(), protocols.end(), [&](const Protocol& p) {
                return p.name == name;
            });
            if (it != protocols.end()) {
                protocols.erase(it);
                saveProtocols();
                ctx.println("Deleted protocol: " + name);
            } else {
                ctx.error("Protocol not found: " + name);
            }
        }
        else if (ctx.checkOpArgs("info", 1, "protocol info [name]")) {
            std::string name = ctx.getOpArg("info", 0);
            for (const auto& p : protocols) {
                if (p.name == name) {
                    std::string statusStr = (p.status == RUNNING ? "RUNNING" :
                                            p.status == IDLE ? "IDLE" : "TERMINATED");
                    ctx.println("Protocol: " + p.name);
                    ctx.println("Status:   " + statusStr);
                    return;
                }
            }
            ctx.error("Protocol not found: " + name);
        }
        else if (ctx.checkOpArgs("run", 1, "protocol run [name]")) {
            std::string name = ctx.getOpArg("run", 0);
            bool found = false;
            for (auto& p : protocols) {
                if (p.name == name) {
                    p.status = RUNNING;
                    found = true;
                    break;
                }
            }
            if (found) {
                saveProtocols();
                ctx.println("Set protocol: " + name + " status to RUNNING");
            } else ctx.error("Protocol not found: " + name);
        }
        else {
            ctx.error("Unknown operation: " + ctx.getArg(0));
        }
    }, "Mainframe protocol operations");
    terminal.addCommand("mainframe", [&](auto& ctx) {
        if(ctx.getClearance() < 20) {
            ctx.error("[Acces DENIED!]: User lacks enough clearance");
            return;
        }
        if (!ctx.checkArgs(1, "mainframe [terminate]")) return;
        if(ctx.is("terminate")) {
            askYesNo(ctx.terminal, "Are you sure you want to terminate?", [&terminal, &app](int result) {
                if (result == 1) {
                    terminal.printDelayedLine("2%", 0.10f * 6.9f);
                    terminal.printDelayedLine("14%", 0.08f * 6.9f);
                    terminal.printDelayedLine("26%", 0.11f * 6.9f);
                    terminal.printDelayedLine("39%", 0.20f * 6.9f);
                    terminal.printDelayedLine("51%", 0.04f * 6.9f);
                    terminal.printDelayedLine("66%", 0.10f * 6.9f);
                    terminal.printDelayedLine("74%", 0.14f * 6.9f);
                    terminal.printDelayedLine("98%", 0.22f * 6.9f);
                    terminal.printDelayedLine("100%", 0.30f * 6.9f);
                    terminal.printDelayedLine("Mainframe termination complete!", 0.50f);
                    terminal.printDelayedLine("Terminating user interface..", 1.50f);
                    terminal.printDelayedLine("Logging out..", 1.50f);
                    terminal.delayedCallback([&app]() {
                        app.setRunning(false);
                    }, 1.50f);
                } else if (result == 0) {
                    return;
                } else {
                    terminal.println("Invalid answer.");
                    return;
                }
            });
        }

    }, "Mainframe power operations");

    // Initial message
    terminal.println("Translucence OS v2.4.1 (Kernel 5.15.0)");
    terminal.println("Welcome back, operative. Type 'help' for instructions.");
    terminal.printPrompt();

    while (app.isRunning()) {
        app.update();
        events.runEvents();

        terminal.setRect(5, 5, (float)app.getWidth() - 10, (float)app.getHeight() - 10);
        terminal.update(app.getDeltaTime());

        renderer.clearBackground(Color::BgDeep);
        terminal.draw(renderer);
        renderer.render();
    }

    return 0;
}
