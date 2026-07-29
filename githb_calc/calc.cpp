/// ===========================================================================
///  Expression Calculator — I/O Layer (入口)
/// ===========================================================================
///  本文件仅负责与用户交互（输入/输出），将核心计算委托给 calc::evaluate()。
///  核心逻辑零耦合于 I/O，可独立复用。
///
///  用法：
///    calc.exe                       交互模式（逐行输入）
///    calc.exe "2+3*4"               计算单个表达式
///    echo 2+3*4 | calc.exe          管道模式（每行一个表达式）

#include "calculator.h"
#include <iostream>
#include <string>
#include <stdexcept>


#ifdef _WIN32
#include <io.h>
#define IS_TTY _isatty
#else
#include <unistd.h>
#define IS_TTY isatty
#endif

// ---- 工具函数 ----------------------------------------------------------------

/// 去除字符串首尾空白
static std::string trim(const std::string& s) {
    std::size_t beg = 0;
    while (beg < s.size() && std::isspace(static_cast<unsigned char>(s[beg]))) ++beg;
    std::size_t end = s.size();
    while (end > beg && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(beg, end - beg);
}

/// 处理并输出一行表达式
/// @return true 若为退出命令
static bool processLine(const std::string& rawLine) {
    std::string line = trim(rawLine);
    if (line.empty()) return false;

    if (line == "quit" || line == "exit" || line == "q") return true;

    try {
        double result = calc::evaluate(line);
        std::cout << "= " << result << "\n";
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return false;
}

// ---- 主入口 ------------------------------------------------------------------

int main(int argc, char* argv[]) {
    // ---- 命令行参数模式：每个参数作为一个表达式 ----
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (i > 1) std::cout << "\n";
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

    // ---- 管道 / 重定向模式（非 TTY）：静默逐行处理 ----
    const bool interactive = IS_TTY(0);  // 0 = STDIN_FILENO

    if (interactive) {
        std::cout << "============================================\n";
        std::cout << "  Expression Calculator  v1.0\n";
        std::cout << "============================================\n";
        std::cout << "  Supported: +  -  *  /  ^  ()  []  {}\n";
        std::cout << "  Examples:  2+3*4   (1+2)^3   2^-3\n";
        std::cout << "  Type 'quit' / 'exit' / 'q' to quit\n";
        std::cout << "--------------------------------------------\n\n";
    }

    std::string line;
    while (true) {
        if (interactive) std::cout << "> ";

        if (!std::getline(std::cin, line)) {
            if (interactive) std::cout << "\n";
            break;   // EOF
        }

        if (processLine(line)) break;
    }

    if (interactive) std::cout << "Goodbye!\n";
    return 0;
}