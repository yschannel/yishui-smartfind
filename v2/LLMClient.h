#pragma once
#include <string>
#include <stdexcept>
#include <cstdlib>

class LLMClient
{
public:
    static std::string getApiKey();
    static std::string sendChatRequest(const std::string &userMessage);
};