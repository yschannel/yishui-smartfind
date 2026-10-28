#pragma once
#include <string>
#include <vector>
#include "SearchQuery.h"
#include "Constants.h"

class NatrualLanguageParser
{
public:
    static SearchQuery parse(const std::string &input);

    // Json from LLM
    static SearchQuery parseFromJson(const std::string &jsonString);

private:
    // static std::string toLower(std::string str);

    // check size keyword
    static bool tryParseSizeKeyword(const std::string &word, SearchQuery &query);

    // check type keyword
    static bool tryParseTypeKeyword(const std::string &word,
                                    SearchQuery &query);

    // transform tool
    static void applyTimeRange(int days, SearchQuery &query);
};