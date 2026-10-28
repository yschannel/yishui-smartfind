#include "FileInfo.h"
#include <cctype>
#include <chrono> //时间处理库
#include <iostream>
#include <algorithm>
#include "Utils.h"
/*
std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}
*/

FileInfo::FileInfo(const std::string &p, uint64_t s, std::filesystem::file_time_type t, const std::string &ext)
    : path(p), size(s), last_write_time(t), extension(ext) {}

FileInfo::FileInfo() : path(""), size(0), last_write_time(std::filesystem::file_time_type::min()), extension("") {}

bool FileInfo::hasExtension(const std::string &ext) const
{
    return toLower(this->extension) == toLower(ext);
}

// 运用系统时钟作为介质时间转换
std::time_t TimeUtils::fileTimeToTimeT(std::filesystem::file_time_type tp)
{
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        tp - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
}

std::filesystem::file_time_type TimeUtils::timeTToFileTime(std::time_t tt)
{
    auto sys_tp = std::chrono::system_clock::from_time_t(tt);

    auto duration = sys_tp - std::chrono::system_clock::now();

    auto file_tp = std::filesystem::file_time_type::clock::now() + duration;

    return file_tp;

    // auto sys_tp = std::chrono::system_clock::from_time_t((tt));
    // return std::filesystem::file_time_type(sys_tp.time_since_epoch());

    // return std::chrono::time_point_cast<std::filesystem::file_time_type::clock::duration>(
    //     std::chrono::system_clock::from_time_t(tt));
}