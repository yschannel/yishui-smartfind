#include "LLMClient.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <vector>

// 链接 WinHTTP 库
#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

// GBK->UTF-8
static std::string gbk_to_utf8(const std::string &gbkStr)
{
    if (gbkStr.empty())
        return "";
    int len = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, nullptr, 0);
    if (len <= 0)
        return gbkStr;
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, &wstr[0], len);

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0)
        return gbkStr;
    std::string utf8Str(utf8Len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], utf8Len, nullptr, nullptr);
    return utf8Str;
}

std::string LLMClient::getApiKey()
{
    const char *apiKey = std::getenv("DEEPSEEK_API_KEY");
    if (apiKey == nullptr)
    {
        throw std::runtime_error("[LLM Error] 未找到环境变量 DEEPSEEK_API_KEY,请先配置!");
    }
    return std::string(apiKey);
}

std::string LLMClient::sendChatRequest(const std::string &userMessage)
{

    std::string safeMessage = gbk_to_utf8(userMessage);
    json requestBody = {
        {"model", "deepseek-chat"},
        {"temperature", 0.1},
        {"messages",
         {{{"role", "system"},
           {"content", "你是一个文件搜索意图解析器。请严格将用户的自然语言转换"
                       "为JSON格式,不要包含任何Markdown标记或额外解释。"
                       "规则:1.只能使用以下字段:'time_range' (格式:xd, xw, "
                       "xm),'file_type'(只能是具体的后缀如 py/cpp "
                       "或通用类别如 image/video),'keywords'"
                       "2.禁止使用 date, type, extension, name 等其他字段。"
                       "3.如果用户没有指定时间，默认不返回 time_range 字段"
                       "4.如果用户没有指定类型，默认不返回 file_type 字段。"
                       "5.必须返回一个 JSON 对象，不要有任何其他解释文字。"
                       "如果用户提到大、小、huge、small等描述文件大小的词,请将其放入 size 字段,不要放入 keywords 字段。"}},
          {{"role", "user"}, {"content", safeMessage}}}}};

    std::string responseStr;
    std::wstring host = L"api.deepseek.com";
    std::wstring path = L"/v1/chat/completions";
    std::string body = requestBody.dump();

    // 从环境变量获取密钥
    char *apiKey = std::getenv("DEEPSEEK_API_KEY");
    if (!apiKey)
    {
        std::cerr << "[LLM Error] 未找到环境变量 DEEPSEEK_API_KEY" << std::endl;
        return "";
    }

    // 1. 初始化 WinHTTP
    HINTERNET hSession = WinHttpOpen(L"Searcher/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return "";

    // 2. 连接服务器
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        return "";
    }

    // 3. 创建请求
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // 4. 设置请求头
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"Authorization: Bearer " + std::wstring(apiKey, apiKey + strlen(apiKey));

    // 5. 发送请求
    BOOL bResult = WinHttpSendRequest(hRequest, headers.c_str(), headers.length(), (LPVOID)body.c_str(), body.length(), body.length(), 0);
    if (bResult)
        bResult = WinHttpReceiveResponse(hRequest, NULL);

    if (bResult)
    {
        // 6. 读取响应数据
        DWORD dwSize = 0;
        std::vector<char> buffer;
        do
        {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                break;
            if (dwSize == 0)
                break;

            buffer.resize(dwSize);
            DWORD dwDownloaded = 0;
            if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded))
                break;

            responseStr.append(buffer.data(), dwDownloaded);
        } while (dwSize > 0);
    }
    else
    {
        std::cerr << "[LLM Error] WinHTTP 请求失败，错误码: " << GetLastError() << std::endl;
    }

    // 7. 清理资源
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // 8. 解析 JSON
    try
    {
        json response = json::parse(responseStr);
        if (response.contains("choices") && !response["choices"].empty())
        {
            std::string content = response["choices"][0]["message"]["content"];
            std::cout << "[LLM] Raw JSON: " << content << std::endl;
            return content;
        }
        else
        {
            std::cerr << "[LLM Error] 响应格式错误: " << responseStr << std::endl;
            return "";
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "[LLM Error] JSON解析失败: " << e.what() << std::endl;
        return "";
    }
}