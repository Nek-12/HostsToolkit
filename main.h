#pragma once
#include <filesystem>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <X11/Xlib.h>

#ifdef __linux__
#define HOSTS "/etc/hosts"
#endif
#ifdef _WIN32
#define HOSTS "C:/Windows/System32/Drivers/etc/hosts"
#endif

class Data {
public:
    static Data& get() {
        static Data instance;
        return instance;
    }
    void update();
    std::vector<std::string> files;
    bool sys_loaded = false;
private:
    Data() = default;
};