#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <vector>

namespace Constants
{
    // file size(type)
    const uint64_t TINY_SIZE = 1024;              // 1KB
    const uint64_t SMALL_SIZE = 1024 * 1024;      // 1MB
    const uint64_t LARGE_SIZE = 1024 * 1024 * 10; // 10MB
    const uint64_t HUGE_SIZE = 1024 * 1024 * 100; // 100MB

    // type cast
    static std::map<std::string, std::vector<std::string>> TYPE_MAP = {
        {"code",
         {".cpp", ".h", ".py", ".java", ".c", ".cs", ".js", ".html", ".css"}},
        {"image",
         {".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff", ".svg", ".webp"}},
        {"video", {".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv"}},
        {"audio", {".mp3", ".wav", ".flac", ".aac", ".ogg"}},
        {"doc",
         {".txt", ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx"}}};

    // time
    const int SECONDS_IN_DAY = 24 * 60 * 60;
}