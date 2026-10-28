#pragma once
#include <string>
#include <vector>
#include "FileInfo.h"
#include "SearchQuery.h"

class Searcher
{
public:
    // mode1: find keyword
    static std::vector<FileInfo> search(const std::vector<FileInfo> &files,
                                        const std::string &keyword);

    // mode2: advanced natural language searching
    static std::vector<FileInfo> search(const std::vector<FileInfo> &files,
                                        const SearchQuery &query);
};