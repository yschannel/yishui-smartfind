#include "Searcher.h"
#include <algorithm>
#include <cctype>

// To lower
static std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return str;
}

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