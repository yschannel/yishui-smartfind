#pragma once
#include <optional>
#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

struct SearchQuery
{
    // 关键词分类
    std::vector<std::string> includeKeywords;
    std::vector<std::string> excludeKeywords;

    // 文件大小范围
    std::optional<uint64_t> minSize;
    std::optional<uint64_t> maxSize;

    // target extensions
    std::vector<std::string> targetExtensions;

    // time range
    std::optional<std::filesystem::file_time_type> minTime;
    std::optional<std::filesystem::file_time_type> maxTime;

    // function: matches
    bool matches(const std::string &path, uint64_t size, const std::string &extension, std::filesystem::file_time_type last_write_time) const;
};