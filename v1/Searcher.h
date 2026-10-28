#pragma once
#include <string>
#include <vector>
#include "FileInfo.h"

class Searcher
{
public:
    // find keyword
    static std::vector<FileInfo> search(const std::vector<FileInfo> &files, const std::string &keyword);
};