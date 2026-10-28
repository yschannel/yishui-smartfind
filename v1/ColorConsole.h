#pragma once
#include <iostream>
#include <filesystem>
#include <windows.h>

enum class Color
{
    _DEFAULT = 7,        // 默认
    _ERROR = 12,         // 亮红
    _SUCCESS = 10,       // 亮绿
    _WARNING = 14,       // 亮黄
    _INFO = 11,          // 亮青
    _PATH_HIGHLIGHT = 15 // 亮白
};

class ColorConsole
{
private:
    static HANDLE hConsole;   // 控制台句柄
    static WORD defaultColor; // 保存默认色

public:
    static HANDLE getHandle()
    {
        if (!hConsole)
        {
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            defaultColor = 7;
        }
        return hConsole;
    }

    // 设置颜色函数
    static void setColor(Color color) { SetConsoleTextAttribute(getHandle(), static_cast<WORD>(color)); }

    // 重置颜色函数
    static void reset() { SetConsoleTextAttribute(getHandle(), defaultColor); }

    // 打印辅助函数
    static void printColored(const std::string &text, Color color)
    {
        setColor(color);
        std::cout << text;
        reset();
    }

    // （针对std::cerr）打印辅助函数
    // 以后面对多个错误再考虑重载
    static void printCerrColored(const std::string &text, const std::filesystem::filesystem_error &ex)
    {
        setColor(Color::_ERROR);
        std::cerr << text << ex.what() << std::endl;
        reset();
    }
    static void printCerrColored(const std::string &text)
    {
        setColor(Color::_ERROR);
        std::cerr << text << std::endl;
        reset();
    }
    static void printCerrColored(const std::string &text1, const std::string &text2)
    {
        setColor(Color::_ERROR);
        std::cerr << text1 << text2 << std::endl;
        reset();
    }
};
