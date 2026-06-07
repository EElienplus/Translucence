#pragma once
#include <Translucence.hpp>
#include "Terminal.hpp"
#include <fstream>
#include <sstream>

struct User {
    std::string name;
    std::string password;
    int clearance = 0;
};

class UserManager {
public:
    UserManager(Terminal& /*t*/) {}

    void addUser(const std::string& name, const std::string& password, int clearance) {
        users[name] = {name, password, clearance};
    }

    bool login(const std::string& name, const std::string& pass) {
        if (users.count(name) && users[name].password == pass) {
            currentUser = users[name];
            isLoggedIn = true;
            return true;
        }
        return false;
    }

    void logout() {
        isLoggedIn = false;
        currentUser = {"guest", "", 0};
    }

    bool loggedIn() const { return isLoggedIn; }
    const User& getCurrentUser() const { return currentUser; }

private:
    std::map<std::string, User> users;
    User currentUser = {"guest", "", 0};
    bool isLoggedIn = false;
};
