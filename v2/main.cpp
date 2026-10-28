#include <algorithm>
#include <iostream>
#include <string>

#include <windows.h> //用于修改控制台编码（支持中文）

#include "Parser.h"
#include "FileInfo.h"
#include "Indexer.h"
#include "Searcher.h"
#include "consoleapi2.h"
#include "ColorConsole.h"
#include "LLMClient.h"

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(65001);

    std::string INDEX_FILE = "default_file_index.bin";
    std::string ROOT_PATH = "D:\\"; // 默认扫描的文件夹

    if (argc > 1)
    {
        // 更改为目前要扫描的文件夹
        ROOT_PATH = argv[1];
        ColorConsole::printColored("[CMD] Root path set to: " + ROOT_PATH + "\n", Color::_INFO);
        std::string safe_root_path = ROOT_PATH;
        std::replace(safe_root_path.begin(), safe_root_path.end(), '\\', '_');
        std::replace(safe_root_path.begin(), safe_root_path.end(), ':', '_');
        INDEX_FILE = safe_root_path + "_file_index.bin";
    }
    else
    {
        // 默认文件夹扫描
        ColorConsole::printColored(
            "[INFO] No path specified, using default: " + ROOT_PATH + "\n",
            Color::_WARNING);

        // 更改文件夹格式提示
        ColorConsole::printColored("[TIP] Usage: Searcher.exe [PATH]\n", Color::_DEFAULT);
    }

    std::vector<FileInfo> fileDatabase;

    // load or construct indexer
    ColorConsole::printColored("Loading index...\n", Color::_DEFAULT);
    fileDatabase = Indexer::loadIndex(INDEX_FILE);

    if (fileDatabase.empty())
    {
        ColorConsole::printColored("Index not found, scanning directory...\n", Color::_INFO);
        fileDatabase = Indexer::scanDirectory(ROOT_PATH);

        if (!fileDatabase.empty())
        {
            ColorConsole::printColored("Index saved successfully.\n", Color::_SUCCESS);
            Indexer::saveIndex(fileDatabase, INDEX_FILE);
        }
        else
        {
            ColorConsole::printColored("Scan result empty or path invalid!\n", Color::_ERROR);
            return 1;
        }
    }

    // IO searcher
    std::string input;
    ColorConsole::printColored("\nIndex Ready! Enter keyword to search (type 'exit' to quit).\n", Color::_SUCCESS);

    while (true)
    {
        ColorConsole::printColored("\n> ", Color::_INFO);
        std::getline(std::cin, input);

        if (input == "exit")
            break;

        SearchQuery query;
        // LLM
        try
        {
            std::string llmJson = LLMClient::sendChatRequest(input);
            if (!llmJson.empty())
            {
                ColorConsole::printColored("[LLM] Raw JSON: " + llmJson + "\n", Color::_INFO);
            }

            query = NatrualLanguageParser::parseFromJson(llmJson);
        }
        catch (const std::exception &e)
        {
            // 容错降级处理
            ColorConsole::printColored(std::string("[WARN] LLM Error: ") + e.what() + ", fallback to local parser.\n", Color::_WARNING);
            query = NatrualLanguageParser::parse(input);
        }

        if (query.includeKeywords.empty() && query.excludeKeywords.empty() && query.targetExtensions.empty())
        {
            ColorConsole::printColored("[Warning] 未能理解您的意图，请尝试更详细的描述，或检查网络连接。\n", Color::_ERROR);
            continue;
        }

        auto results = Searcher::search(fileDatabase, query);

        ColorConsole::printColored("\n--- Search Results (" + std::to_string(results.size()) + " found) ---\n", Color::_INFO);

        // print the first 20 results.
        int limit = std::min(20, (int)results.size());
        for (int i = 0; i < limit; ++i)
        {
            std::cout << "[" << std::setw(2);
            ColorConsole::printColored(std::to_string(i + 1), Color::_DEFAULT);
            std::cout << "] (" << std::setw(6);

            ColorConsole::printColored(std::to_string(results[i].size), Color::_SUCCESS);
            std::cout << " bytes) ";

            // Path highlight
            ColorConsole::printColored(results[i].path + "\n", Color::_PATH_HIGHLIGHT);
        }
        if (results.size() > 20)
        {
            ColorConsole::printColored("... and " + std::to_string(results.size() - 20) + " more results hidden", Color::_WARNING);
        }
    }

    return 0;
}