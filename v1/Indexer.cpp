#include "Indexer.h"
#include "FileInfo.h"
#include "ColorConsole.h"
#include <fstream>
#include <iostream>
#include <filesystem>

std::vector<FileInfo> Indexer::scanDirectory(const std::string &rootPath)
{
    std::vector<FileInfo> files;

    // 检查文件是否存在
    if (!std::filesystem::exists(rootPath))
    {
        ColorConsole::printCerrColored("[ERROR] PATH NOT EXISTS: ", rootPath);
        // std::cerr << "[ERROR] PATH NOT EXISTS: " << rootPath << std::endl;
        return files;
    }

    try
    {
        // recursive_directory_iterator 自动递归进入子文件夹
        // skip_permission_denied 遇到没有权限的文件夹直接跳过，不要报错
        for (const auto &entry : std::filesystem::recursive_directory_iterator(
                 rootPath, std::filesystem::directory_options::skip_permission_denied))
        {

            // 只处理普通文件（skip 文件夹、链接······）
            if (entry.is_regular_file())
            {
                try
                {
                    // get fileInfo
                    std::string path = entry.path().string();
                    uint64_t size = entry.file_size();
                    auto time = entry.last_write_time();

                    files.emplace_back(path, size, time);
                }
                catch (const std::exception &e)
                {
                    // ignore read error of individual documents
                    ColorConsole::setColor(Color::_WARNING);
                    std::cerr
                        << "[WARNING] FAILED TO READ FILEINFO!" << entry.path().string()
                        << " | Reason: " << e.what() << std::endl;
                    ColorConsole::reset();

                    // ColorConsole::printCerrColored("[WARNING] FAILED TO READ FILEINFO!", entry.path().string());
                    // ColorConsole::printCerrColored(" | Reason: ", e.what());

                    /*std::cerr
                        << "[WARNING] FAILED TO READ FILEINFO!" << entry.path().string()
                        << " | Reason: " << e.what() << std::endl;
                        */
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error &ex)
    {
        ColorConsole::printCerrColored("[ERROR] ANOMALY DETECTED DURING SCAN: ", ex);
    }

    ColorConsole::setColor(Color::_SUCCESS);
    std::cout << "[COMPLETED] SCAN COMPLETED, found " << files.size() << " file(s)." << std::endl;
    ColorConsole::reset();

    return files;
}

bool Indexer::saveIndex(const std::vector<FileInfo> &files, const std::string &filename)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        ColorConsole::printCerrColored("[ERROR] CANNOT OPEN FILE FOR WRITING: ", filename);
        // std::cerr << "[ERROR] CANNOT OPEN FILE FOR WRITING: " << filename << std::endl;
        return false;
    }

    // 写入文件总数
    uint64_t count = static_cast<uint64_t>(files.size());
    outFile.write(reinterpret_cast<const char *>(&count), sizeof(count)); // 强制类型转换成char*, 直接搬运内存字节

    // 遍历写入文件
    for (const auto &file : files)
    {
        // pathLen
        uint64_t pathLen = static_cast<uint64_t>(file.path.size());
        outFile.write(reinterpret_cast<const char *>(&pathLen), sizeof(pathLen));
        // content
        if (pathLen > 0)
        {
            outFile.write(file.path.c_str(), static_cast<std::streamsize>(pathLen));
        }
        // size
        outFile.write(reinterpret_cast<const char *>(&file.size), sizeof(file.size));
        // time
        std::time_t tt = TimeUtils::fileTimeToTimeT(file.last_write_time);
        int64_t timeVal = static_cast<int64_t>(tt);
        outFile.write(reinterpret_cast<const char *>(&timeVal), sizeof(timeVal));
    }

    return outFile.good();
}

std::vector<FileInfo> Indexer::loadIndex(const std::string &filename)
{
    std::vector<FileInfo> files;
    std::ifstream inFile(filename, std::ios::binary);

    if (!inFile)
    {
        return files; // 返空
    }

    // 总数
    uint64_t count = 0;
    inFile.read(reinterpret_cast<char *>(&count), sizeof(count));

    if (!inFile)
    {
        ColorConsole::printCerrColored("[ERROR] INDEX FILE CORRUPTION: UNABLE TO READ FILE HEADER.");
        /*std::cerr
            << "[ERROR] INDEX FILE CORRUPTION: UNABLE TO READ FILE HEADER."
            << std::endl;
        */
    }

    // 安全检查：防止因为文件损坏导致内存溢出
    const uint64_t MAX_VALID_FILES = 10000000;
    if (count > MAX_VALID_FILES)
    {
        ColorConsole::printCerrColored("[ERROR] INDEX FILE CORRUPTED: Invalid file count detected.");
        return std::vector<FileInfo>(); // 返空
    }

    files.reserve(count);

    // 读取
    for (uint64_t i = 0; i < count; ++i)
    {
        uint64_t pathLen = 0;
        inFile.read(reinterpret_cast<char *>(&pathLen), sizeof(pathLen));

        if (!inFile)
            break;

        // 内容
        std::string path(pathLen, '\0');
        inFile.read(&path[0], static_cast<std::streamsize>(pathLen));

        if (!inFile)
            break;

        // 大小
        uint64_t size = 0;
        inFile.read(reinterpret_cast<char *>(&size), sizeof(size));

        // 时间
        int64_t tempTime = 0;
        inFile.read(reinterpret_cast<char *>(&tempTime), sizeof(tempTime));

        if (!inFile)
            break;

        std::time_t tt = static_cast<std::time_t>(tempTime);

        auto ftime = TimeUtils::timeTToFileTime(tt);

        files.emplace_back(path, size, ftime);
    }

    if (files.size() != count)
    {
        ColorConsole::setColor(Color::_ERROR);
        std::cerr << "[Warning] Index file read incomplete! Expected: " << count << ", Actual: " << files.size() << std::endl;
        ColorConsole::reset();
        // std::cerr << "[Warning] Index file read incomplete! Expected: " << count << ", Actual: " << files.size() << std::endl;
    }
    else
    {
        ColorConsole::setColor(Color::_SUCCESS);
        std::cout << "[LOAD] Successfully loaded " << files.size() << " file records from " << filename << std::endl;
        ColorConsole::reset();
        // std::cout << "[LOAD] Successfully loaded" << files.size() << " file records from " << filename << std::endl;
    }

    return files;
}
