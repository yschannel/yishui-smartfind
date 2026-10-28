#include "Parser.h"
#include "Constants.h"
#include "FileInfo.h"
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <charconv>
#include "Utils.h"

using json = nlohmann::json;

// tool

void NatrualLanguageParser::applyTimeRange(int days, SearchQuery &query)
{
    auto now = std::chrono::system_clock::now();
    auto target_sys_time = now - std::chrono::hours(24 * days);
    std::time_t target_time_t =
        std::chrono::system_clock::to_time_t(target_sys_time);
    query.minTime = TimeUtils::timeTToFileTime(target_time_t);
}
/*
std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}
*/

static void resolveExtensions(const std::string &typeKeyword,
                              std::vector<std::string> &outExtensions)
{
    std::string lowerKey = toLower(typeKeyword);

    auto it = Constants::TYPE_MAP.find(lowerKey);
    if (it != Constants::TYPE_MAP.end())
    {
        outExtensions.insert(outExtensions.end(), it->second.begin(),
                             it->second.end());
        return;
    }

    if (!lowerKey.empty())
    {
        if (lowerKey[0] != '.')
        {
            outExtensions.push_back("." + lowerKey);
        }
        else
        {
            outExtensions.push_back(lowerKey);
        }
    }
}

std::filesystem::file_time_type daysAgo(int days)
{
    auto now = std::chrono::system_clock::now();

    auto target_time =
        now - std::chrono::seconds(static_cast<int64_t>(days) *
                                   Constants::SECONDS_IN_DAY);

    std::time_t target_time_t = std::chrono::system_clock::to_time_t(target_time);

    return TimeUtils::timeTToFileTime(target_time_t);
}

bool NatrualLanguageParser::tryParseSizeKeyword(const std::string &word, SearchQuery &query)
{
    std::string lowerWord = toLower(word);

    if (lowerWord == "tiny" || lowerWord == "small")
    {
        query.maxSize = Constants::SMALL_SIZE;
        return true;
    }
    else if (lowerWord == "large" || lowerWord == "big")
    {
        query.minSize = Constants::SMALL_SIZE;
        query.maxSize = Constants::LARGE_SIZE;
        return true;
    }
    else if (lowerWord == "huge" || lowerWord == "giant")
    {
        query.minSize = Constants::LARGE_SIZE;
        return true;
    }

    return false;
}

bool NatrualLanguageParser::tryParseTypeKeyword(const std::string &word, SearchQuery &query)
{
    std::string lowerword = toLower(word);

    for (const auto &pair : Constants::TYPE_MAP)
    {
        if (lowerword == pair.first)
        {
            for (const auto &ext : pair.second)
            {
                query.targetExtensions.push_back(ext);
            }
            return true;
        }
    }

    return false;
}

// LLM Json
/*
SearchQuery NatrualLanguageParser::parseFromJson(const std::string &jsonString)
{
    SearchQuery query;

    try
    {
        json j = json::parse(jsonString);

        // time range
        if (j.contains("time_range"))
        {
            std::string range = j["time_range"];

            int num;
            auto [ptr, ec] =
                std::from_chars(range.data(), range.data() + range.size(), num);

            if (ec == std::errc() && ptr != range.data() + range.size())
            {
                char suffix = *ptr;
                if (suffix == 'd')
                {
                    applyTimeRange(num, query);
                }
            }
        }

        // file type
        if (j.contains("file_type"))
        {
            std::string ext = j["file_type"];
            std::cout << "[DEBUG] LLM suggested extension: " << ext <<
std::endl; if (!ext.empty() && ext[0] != '.') ext = "." + ext;
            query.targetExtensions.push_back(ext);
        }

        // size
        if (j.contains("size_constraint"))
        {
            std::string size = j["size_constraint"];
            if (size == "small" || size == "tiny")
            {
                query.maxSize = Constants::SMALL_SIZE;
            }
            else if (size == "large" || size == "big")
            {
                query.minSize = Constants::SMALL_SIZE;
                query.maxSize = Constants::LARGE_SIZE;
            }
            else if (size == "huge")
            {
                query.minSize = Constants::LARGE_SIZE;
            }
        }

        // keywords
        if (j.contains("keywords"))
        {
            for (auto &kw : j["keywords"])
            {
                query.includeKeywords.push_back(kw.get<std::string>());
            }
        }
    }
    catch (json::parse_error &e)
    {
        // 失败降级
        std::cerr << "[Parser Error] Failed to parse LLM JSON: " << e.what()
                  << std::endl;
        std::cerr << "[Parser Error] Input JSON was: " << jsonString <<
std::endl;
    }

    return query;
}
*/

SearchQuery
NatrualLanguageParser::parseFromJson(const std::string &jsonString)
{
    SearchQuery query;

    if (jsonString.empty() || jsonString.find_first_not_of(" \t\n\r") == std::string::npos)
    {
        std::cerr << "[Parser Warn] Empty or whitespace JSON input." << std::endl;
        return query;
    }

    try
    {
        json j = json::parse(jsonString);

        // time range
        if (j.contains("time_range") || j.contains("date") || j.contains("days"))
        {
            try
            {
                auto &timeNode = j.contains("time_range") ? j["time_range"] : (j.contains("date") ? j["date"] : j["days"]);

                int targetDays = -1;

                if (timeNode.is_string())
                {
                    std::string timeStr = timeNode;

                    std::string numStr;
                    for (char c : timeStr)
                    {
                        if (std::isdigit(c))
                            numStr += c;
                    }
                    if (!numStr.empty())
                    {
                        targetDays = std::stoi(numStr);
                    }
                }
                else if (timeNode.is_number_integer())
                {
                    targetDays = timeNode;
                }

                if (targetDays > 0)
                {
                    query.minTime = daysAgo(targetDays);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[Parser Warn] 时间解析异常: " << e.what() << std::endl;
            }
        }

        // file_type > type > extension
        std::string typeKey;

        if (j.contains("file_type"))
            typeKey = "file_type";
        else if (j.contains("type"))
            typeKey = "type";
        else if (j.contains("extension"))
            typeKey = "extension";

        if (!typeKey.empty())
        {
            try
            {
                auto &typeNode = j[typeKey];

                auto processType =
                    [&query](const std::string &typeStr)
                {
                    std::vector<std::string> tempExts;
                    resolveExtensions(typeStr, tempExts);
                    // 去重
                    for (const auto &ext : tempExts)
                    {
                        bool exists = std::find_if(query.targetExtensions.begin(), query.targetExtensions.end(), [&ext](const std::string &s)
                                                   { return s == ext; }) != query.targetExtensions.end();

                        if (!exists)
                        {
                            query.targetExtensions.push_back(ext);
                        }
                        /*
                            if (std::find(query.targetExtensions.begin(),
                                          query.targetExtensions.end(),
                                          ext) == query.targetExtensions.end())
                            {
                                query.targetExtensions.push_back(ext);
                            }
                          */
                    }
                };

                // string
                if (typeNode.is_string())
                {
                    processType(typeNode);
                }
                else if (typeNode.is_array())
                {
                    for (auto &item : typeNode)
                    {
                        if (item.is_string())
                        {
                            processType(item);
                        }
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[Parser Warn] 类型解析异常: " << e.what() << std::endl;
            }
        }

        // size
        if (j.contains("size_constraint") || j.contains("size"))
        {
            std::string sizeKey = j.contains("size_constraint") ? "size_constraint" : "size";
            std::string size = j[sizeKey];
            std::string lowerSize = toLower(size);

            if (lowerSize == "small" || lowerSize == "tiny")
            {
                query.maxSize = Constants::SMALL_SIZE;
            }
            else if (lowerSize == "large" || lowerSize == "big")
            {
                query.minSize = Constants::SMALL_SIZE;
                query.maxSize = Constants::LARGE_SIZE;
            }
            else if (lowerSize == "huge")
            {
                query.minSize = Constants::LARGE_SIZE;
            }
        }

        // Keywords
        if (j.contains("keywords") || j.contains("keyword"))
        {
            std::string kwKey = j.contains("keywords") ? "keywords" : "keyword";
            try
            {
                auto &kwNode = j[kwKey];
                if (kwNode.is_array())
                {
                    for (auto &kw : kwNode)
                    {
                        if (kw.is_string() && !kw.empty())
                        {
                            query.includeKeywords.push_back(kw);
                        }
                    }
                }
                else if (kwNode.is_string())
                {

                    if (!kwNode.empty())
                    {
                        query.includeKeywords.push_back(kwNode);
                    }
                }
            }
            catch (...)
            {
                std::cerr << "[Parser Warn] 关键词解析异常" << std::endl;
            }
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "[Parser Error] JSON 语法错误: " << e.what() << std::endl;
        std::cerr << "[Parser Error] 错误输入: " << jsonString << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Parser Error] 未知异常: " << e.what() << std::endl;
    }

    std::cout << "[DEBUG] 解析结果: 扩展名=" << query.targetExtensions.size()
              << ", 关键词=" << query.includeKeywords.size()
              << ", 时间限制=" << query.minTime.has_value() << std::endl;

    return query;
}

// function: major analysis
SearchQuery NatrualLanguageParser::parse(const std::string &input)
{
    SearchQuery query;
    std::string lowerinput = toLower(input);

    // simple tokenization: splitting by spaces
    std::istringstream iss(lowerinput);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token)
    {
        tokens.push_back(token);
    }

    // logic: "without" "exclude"
    bool expectExclude = false;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string &word = tokens[i];

        if (word == "without" || word == "exclude" || word == "no")
        {
            expectExclude = true;
            continue;
        }

        if (expectExclude)
        {
            query.excludeKeywords.push_back(word);
            expectExclude = false;
            continue;
        }

        if (tryParseSizeKeyword(word, query))
        {
            continue;
        }

        if (tryParseTypeKeyword(word, query))
        {
            continue;
        }

        // time
        if (word == "today" || word == "recent" || word == "new")
        {
            query.minTime = daysAgo(0);
            continue;
        }
        if (word == "yesterday")
        {
            query.minTime = daysAgo(1);
            query.maxTime = daysAgo(0);
            continue;
        }

        bool isNumber = !word.empty() && std::all_of(word.begin(), word.end(), [](unsigned char c)
                                                     { return std::isdigit(c); });
        if (isNumber && i + 1 < tokens.size())
        {
            std::string nextWord = tokens[i + 1];

            if (nextWord == "days" || nextWord == "day")
            {
                int num = std::stoi(word);
                if (num > 0)
                {
                    query.minTime = daysAgo(num);
                }
                ++i;

                if (i + 1 < tokens.size() && tokens[i + 1] == "ago")
                {
                    ++i;
                }

                continue;
            }
        }

        //"not xxx"
        if (word == "not" && i + 1 < tokens.size())
        {
            query.excludeKeywords.push_back(tokens[++i]);
        }
        else
        {
            query.includeKeywords.push_back(word);
        }
    }

    // DEBUG
    std::cout << "[DEBUG] Parsed Query: Include=" << query.includeKeywords.size()
              << ", Exclude=" << query.excludeKeywords.size()
              << ", MinTime=" << query.minTime.has_value()
              << std::endl;

    return query;
}