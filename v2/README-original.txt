Yshui-SmartFind

技术栈

语言标准：C++17
开发环境：VS Code + MinGW-w64
平台：Windows 11


编译与安装

本项目直接通过命令行编译。

编译步骤
克隆或下载本仓库。
在终端中进入项目目录。
使用以下命令编译（以 g++ 为例）：

 g++ -std=c++17 -I./third_party *.cpp -o Searcher.exe -lwinhttp
（注：如果你使用 VS Code，请确保 .vscode/tasks.json 中配置了 C++17 标准。）

输入个人DeepSeek密钥
$env:DEEPSEEK_API_KEY="[你的密钥]"; 

使用方法

基本运行
直接在命令行运行程序，默认扫描 D: 盘：

 .\Searcher.exe

指定扫描路径
你可以通过命令行参数指定需要建立索引的目录：

 .\Searcher.exe "E:\MyProject"


输入 exit 退出程序。