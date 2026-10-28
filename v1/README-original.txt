Yshui-SmartFind

一个基于 C++17 开发的高性能本地文件检索工具。它通过预构建二进制索引，实现毫秒级的文件搜索体验，并支持命令行交互与彩色控制台输出。

技术栈

语言标准：C++17
开发环境：VS Code + MinGW-w64
平台：Windows 11

编译与安装

本项目直接通过命令行编译。

前置要求
支持 C++17 的编译器 (GCC 8.1+, Clang 7+, MSVC 2019+)

编译步骤
克隆或下载本仓库。
在终端中进入项目目录。
使用以下命令编译（以 g++ 为例）：

g++ -std=c++17 *.cpp -o Searcher.exe

注：如果你使用 VS Code，请确保 .vscode/tasks.json 中配置了 C++17 标准。

使用方法

基本运行
直接在命令行运行程序，默认扫描 D: 盘：

 .\Searcher.exe

指定扫描路径
你可以通过命令行参数指定需要建立索引的目录：

 .\Searcher.exe E:\MyProject

搜索操作
程序启动并加载索引后，进入交互模式：
在 >  提示符后输入关键词（例如 .cpp 或 report）。
程序会立即列出匹配的文件路径、大小和修改时间。
输入 exit 退出程序。

项目结构

Yshui-SmartFind/
├── main.cpp            # 程序入口与主控制逻辑
├── Indexer.h/cpp       # 目录扫描与索引持久化模块
├── Searcher.h/cpp      # 搜索算法实现
├── FileInfo.h/cpp      # 文件数据结构定义
├── ColorConsole.h/cpp  # 控制台颜色美化封装
└── README.txt        # 项目说明文档