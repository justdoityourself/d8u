/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <filesystem>

#include "util.hpp"

namespace d8u
{
    std::string execute(const char* cmd) 
    {
        std::cout << cmd << std::endl;

        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);

        if (!pipe) 
            throw std::runtime_error("popen() failed!");
        
        while (fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();

        return result;
    }

    void powershell(const std::string & command)
    {
        std::cout << command << std::endl;
        system((std::string("start powershell.exe ") + command).c_str());
    }

    void cmd(const std::string & command)
    {
        std::cout << command << std::endl;
        system(command.c_str());
    }

    void diskpart(const std::string& command)
    {
        util::string_as_file("system_command.txt", command);

        system("diskpart /s system_command.txt");

        std::filesystem::remove("system_command.txt");
    }

    void mount(std::string_view disknumber, std::string_view driveletter)
    {
        return diskpart(std::string("select disk ") + std::string(disknumber) + "\nselect partition 1\nassign letter=" + std::string(driveletter) + "\n");
    }

    void partition(std::string_view disknumber, std::string_view driveletter)
    {
        return diskpart(std::string("select disk ") + std::string(disknumber) + "\ncreate partition primary\nselect partition 1\nassign letter=" + std::string(driveletter) + "\nselect volume=" + std::string(driveletter) + "\nformat quick\n");
    }
}





