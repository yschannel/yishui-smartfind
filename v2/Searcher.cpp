#include "Searcher.h"
#include "SearchQuery.h"
#include <algorithm>
#include <cctype>
#include "Utils.h"

// To lower
/*
static std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return str;
}
*/

std::vector<FileInfo> Searcher::search(const std::vector<FileInfo> &files,
                                       const std::string &keyword)
{
    std::vector<FileInfo> results;
    std::string lowerKeyword = toLower(keyword);

    for (const auto &file : files)
    {
        if (toLower(file.path).find(lowerKeyword) != std::string::npos)
        {
            results.push_back(file);
        }
    }

    return results;
}

bool SearchQuery::matches(
    const std::string &path, uint64_t size, const std::string &extension,
    std::filesystem::file_time_type last_write_time) const
{

    // check keywords
    std::string lowerPath = toLower(path);

    for (const auto &word : includeKeywords)
    {
        if (lowerPath.find(toLower(word)) == std::string::npos)
        {
            return false;
        }
    }

    for (const auto &word : excludeKeywords)
    {
        if (lowerPath.find(toLower(word)) != std::string::npos)
        {
            return false;
        }
    }

    // check size
    if (minSize.has_value() && size < minSize.value())
        return false;
    if (maxSize.has_value() && size > maxSize.value())
        return false;

    // check type
    if (!targetExtensions.empty())
    {
        bool matched = false;
        std::string lowerExt = toLower(extension);
        for (const auto &ext : targetExtensions)
        {
            if (lowerExt == toLower(ext))
            {
                matched = true;
                break;
            }
        }
        if (!matched)
            return false;
    }

    // check time
    auto file_time_t = TimeUtils::fileTimeToTimeT(last_write_time);

    if (minTime.has_value())
    {
        auto min_time_t = TimeUtils::fileTimeToTimeT(minTime.value());
        if (file_time_t < min_time_t)
            return false;
    }

    if (maxTime.has_value())
    {
        auto max_time_t = TimeUtils::fileTimeToTimeT(maxTime.value());
        if (file_time_t > max_time_t)
            return false;
    }

    return true;
}

std::vector<FileInfo> Searcher::search(const std::vector<FileInfo> &files,
                                       const SearchQuery &query)
{
    std::vector<FileInfo> results;

    for (const auto &file : files)
    {
        if (query.matches(file.path, file.size, file.extension, file.last_write_time))
        {
            results.push_back(file);
        }
    }

    return results;
}