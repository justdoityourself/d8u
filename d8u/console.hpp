/* Copyright (C 2022 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <iostream>
#include <thread>

namespace d8u
{
    const char* White = "[0m";
    const char * Red =       "[0;31m";
    const char * Green =     "[0;32m";
    const char * Blue =      "[0;34m";
    const char * Cyan =      "[0;36m";
    const char * Yellow =    "[0;33m";
    const char * Grey =      "[1;30m";

    const char * LightGrey =     "[0;37m";
    const char * BrightRed =     "[1;31m";
    const char * BrightGreen =   "[1;32m";
    const char * BrightWhite =   "[1;37m";
    const char * BrightYellow =  "[1;33m";

    std::string operator "" _red(const char* c, size_t l) {
        return std::string("\033") + Red + c + std::string("\033") + White;
    }
    std::string operator "" _green(const char* c, size_t l) {
        return std::string("\033") + Green + c + std::string("\033") + White;
    }
    std::string operator "" _blue(const char* c, size_t l) {
        return std::string("\033") + Blue + c + std::string("\033") + White;
    }
    std::string operator "" _cyan(const char* c, size_t l) {
        return std::string("\033") + Cyan + c + std::string("\033") + White;
    }
    std::string operator "" _yellow(const char* c, size_t l) {
        return std::string("\033") + Yellow + c + std::string("\033") + White;
    }
    std::string operator "" _grey(const char* c, size_t l) {
        return std::string("\033") + Grey + c + std::string("\033") + White;
    }

    std::string operator "" _lgrey(const char* c, size_t l) {
        return std::string("\033") + LightGrey + c + std::string("\033") + White;
    }
    std::string operator "" _bred(const char* c, size_t l) {
        return std::string("\033") + BrightRed + c + std::string("\033") + White;
    }
    std::string operator "" _bgreen(const char* c, size_t l) {
        return std::string("\033") + BrightGreen + c + std::string("\033") + White;
    }
    std::string operator "" _bwhite(const char* c, size_t l) {
        return std::string("\033") + BrightWhite + c + std::string("\033") + White;
    }
    std::string operator "" _byellow(const char* c, size_t l) {
        return std::string("\033") + BrightYellow + c + std::string("\033") + White;
    }

    std::string yellow(const auto& c) {
        return std::string("\033") + Yellow + std::string(c) + std::string("\033") + White;
    }

    std::string green(const auto& c) {
        return std::string("\033") + Green + std::string(c) + std::string("\033") + White;
    }

    std::string bgreen(const auto& c) {
        return std::string("\033") + BrightGreen + std::string(c) + std::string("\033") + White;
    }

    std::string bred(const auto& c) {
        return std::string("\033") + BrightRed + std::string(c) + std::string("\033") + White;
    }

    std::string bwhite(const auto& c) {
        return std::string("\033") + BrightWhite + std::string(c) + std::string("\033") + White;
    }

    void ConsoleColor(const char* color)
    {
        std::cout << '\033' << color;
    }

    void WithColor(const char* color, auto f) {
        ConsoleColor(color);

        f();

        ConsoleColor(White);
    }

    void Clear()
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear"); 
#endif
    }

    void WithRedraw(auto f)
    {
        Clear();
        f();
    }

    void CountDown(size_t n)
    {
        for (size_t i = n; i > 0; i--) {
            std::cout << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void Render(auto f,size_t i=1)
    {
        while (true) 
        {
            WithRedraw(f);
            std::this_thread::sleep_for(std::chrono::seconds(i));
        }
    }
}