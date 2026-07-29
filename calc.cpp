// ============================================================================
//  calc.cpp — I/O 层（用户交互入口）
// ============================================================================
//  本文件是"程序的入口"，但它只负责和用户打交道：
//    读用户的输入 → 调用核心计算 → 把结果（或错误）打印出来
//
//  它完全不知道 Lexer、Parser、AST 的存在，
//  只知道 calc::evaluate() 这一个函数。
//  这就是"关注点分离"——核心逻辑可以脱离界面独立存在。
//
//  三种运行模式：
//    1. 命令行参数：  ecalc "2+3*4" "5^2"
//       每个参数当成一个表达式，逐个计算并打印
//
//    2. 交互模式：    ecalc（不带参数，在真正的终端里运行）
//       显示提示符 > ，逐行读取，逐行计算
//       输入 quit / exit / q 退出
//
//    3. 管道模式：    echo 2+3*4 | ecalc
//       静默逐行读取，不显示提示符和 banner
//       通过 isatty() 检测标准输入是否连接了终端来区分模式 2 和 3

#include "calculator.h"
#include <iostream>
#include <string>
#include <stdexcept>

// ---- 跨平台的终端检测 ----
// Windows 用 _isatty，Linux/macOS 用 isatty
// 如果标准输入连着真正的终端 → 交互模式；否则 → 管道模式
#ifdef _WIN32
#include <io.h>
#define IS_TTY _isatty
#else
#include <unistd.h>
#define IS_TTY isatty
#endif

// =========================================================================
//  工具函数
// =========================================================================

// 去掉字符串两端的空白字符（空格、Tab 等）
// 例如 trim("  hello  ") → "hello"
static std::string trim(const std::string& s) {
    std::size_t beg = 0;
    while (beg < s.size() && std::isspace(static_cast<unsigned char>(s[beg])))
        ++beg;
    std::size_t end = s.size();
    while (end > beg && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;
    return s.substr(beg, end - beg);
}

// 处理一行输入：去掉空白、判断是否退出命令、调用核心计算
// 返回 true 表示用户想退出
static bool processLine(const std::string& rawLine) {
    std::string line = trim(rawLine);
    if (line.empty()) return false;   // 空行，跳过

    // 退出命令（不区分大小写靠 == 做不到，但 quit/QUIT 都行就算了）
    if (line == "quit" || line == "exit" || line == "q")
        return true;

    // 调用核心纯函数 calc::evaluate()
    try {
        double result = calc::evaluate(line);
        std::cout << "= " << result << "\n";
    } catch (const std::runtime_error& e) {
        // 错误消息打印到 stderr，和正常输出区分开（管道场景下有用）
        std::cerr << "Error: " << e.what() << "\n";
    }
    return false;
}

// =========================================================================
//  主入口
// =========================================================================
int main(int argc, char* argv[]) {
    // ---- 模式 1：命令行参数 ----
    // 例如 ecalc "1+2" "3*4"
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (i > 1) std::cout << "\n";   // 多个表达式之间空一行
            std::cout << "> " << argv[i] << "\n";
            try {
                double result = calc::evaluate(argv[i]);
                std::cout << "= " << result << "\n";
            } catch (const std::runtime_error& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
        }
        return 0;
    }

    // ---- 判断是交互模式还是管道模式 ----
    // IS_TTY(0) 检查文件描述符 0（标准输入）是不是终端
    const bool interactive = IS_TTY(0);

    // 交互模式：打印欢迎信息
    if (interactive) {
        std::cout << "============================================\n";
        std::cout << "  Expression Calculator  v1.0\n";
        std::cout << "============================================\n";
        std::cout << "  Supported: +  -  *  /  ^  ()  []  {}\n";
        std::cout << "  Examples:  2+3*4   (1+2)^3   2^-3\n";
        std::cout << "  Type 'quit' / 'exit' / 'q' to quit\n";
        std::cout << "--------------------------------------------\n\n";
    }

    // ---- 主循环：逐行读取 → 处理 ----
    std::string line;
    while (true) {
        if (interactive) std::cout << "> ";   // 交互模式显示提示符

        // 读一行；如果 EOF（Ctrl+Z / Ctrl+D）则退出
        if (!std::getline(std::cin, line)) {
            if (interactive) std::cout << "\n";
            break;
        }

        if (processLine(line)) break;   // 用户输入了退出命令
    }

    if (interactive) std::cout << "Goodbye!\n";
    return 0;
}
