# Yishui SmartFind

使用 C++17 编写的 Windows 本地文件检索课程项目，保留 1.0 与 2.0 两个版本。

| 目录 | 内容 |
| --- | --- |
| `v1/` | 文件扫描、二进制索引、关键词检索、彩色控制台输出 |
| `v2/` | 在基础检索上增加自然语言查询解析与 DeepSeek 接口调用 |

## 编译

需要 Windows 和支持 C++17 的 MinGW-w64 / w64devkit。请分别在对应版本目录中编译，不要将两个版本一起链接。

```bash
# v1 目录
g++ -std=c++17 *.cpp -o Searcher.exe

# v2 目录
g++ -std=c++17 -I./third_party *.cpp -o Searcher.exe -lwinhttp
```

`v2/third_party/` 保留原项目附带的 nlohmann/json 与 cpp-httplib 头文件及其版权说明。当前模型请求使用 Windows WinHTTP。

## 使用

在 PowerShell 中运行，并显式指定要索引的文件夹：

```powershell
.\Searcher.exe "D:\SampleFiles"
```

原程序不传参数时默认扫描 `D:\`。首次运行会创建索引文件，后续读取已有索引；索引包含本地文件路径，不要上传或分享个人索引。输入 `exit` 退出。

v1 接受关键词；v2 会将输入的自然语言查询发送给 DeepSeek。v2 需先在当前 PowerShell 会话配置自己的密钥：

```powershell
$env:DEEPSEEK_API_KEY = "YOUR_API_KEY"
.\Searcher.exe "D:\SampleFiles"
```

此处是占位符，本仓库不包含真实密钥。API 调用可能产生服务费用。原程序包含本地解析逻辑，但不同接口失败情形下的降级行为尚未全面验证。

## 验证

归档时使用 Windows 上的 w64devkit GCC 15.2.0，按上述命令成功编译并链接了 v1 和 v2。未调用线上 API，未进行完整检索功能测试。

## 归档说明

- 保留两版源码和原始使用说明；内容一致的实验提交包副本已去重。
- 排除本机文件索引、可执行文件、实验报告、压缩包和 IDE 配置。
- 提交使用 GitHub noreply 邮箱；上传副本已检查姓名、学号、本机用户路径和常见密钥格式。
- 保留课程代码原有实现，未宣称全部功能已经验证。
