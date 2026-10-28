#pragma once
#include <string>
#include <vector>
#include <filesystem>

struct FileInfo
{
    std::string path;
    uint64_t size; // 文件大小统一用uint64_t
    // 记录文件最后修改时间
    std::filesystem::file_time_type last_write_time;

    FileInfo(const std::string &p, uint64_t s, std::filesystem::file_time_type t);
    FileInfo();
};

// tool: 处理时间转换(C++17)(文件时间与标准时间)
class TimeUtils
{
public:
    // file_time -> second
    static std::time_t fileTimeToTimeT(std::filesystem::file_time_type tp);
    // second -> file_time
    static std::filesystem::file_time_type timeTToFileTime(std::time_t tt);
};