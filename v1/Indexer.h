#pragma once
#include <string>
#include <vector>
#include "FileInfo.h"

class Indexer
{
public:
    // scan Directory and return rootpath
    static std::vector<FileInfo> scanDirectory(const std::string &rootPath);

    // save index to 2d doc
    static bool saveIndex(const std::vector<FileInfo> &files, const std::string &filename);

    // load index from 2d doc and
    static std::vector<FileInfo> loadIndex(const std::string &filename);
};