// ============================================================================
//  calculator.cpp — 核心 API 的实现（三级流水线）
// ============================================================================
//  这是整个库的"调度中心"。evaluate() 函数串联了三个独立模块，
//  形成一条清晰的流水线：
//
//    输入字符串
//       │
//       ▼
//   [第一阶段] Lexer（词法分析）
//       把 "2+3*4" 切成 Token 序列: Number(2), Plus, Number(3), Multiply, Number(4), End
//       │
//       ▼
//   [第二阶段] Parser（语法分析）
//       把 Token 序列组织成一棵表达式树：
//         加法是根，左子=2，右子=乘法，乘法下面=3和4
//       │
//       ▼
//   [第三阶段] AST::evaluate（求值）
//       递归计算表达式树的值 → 14
//       │
//       ▼
//     返回 double 结果（或抛出 runtime_error）
//
//  为什么分成三个阶段？
//    - 每个阶段只做一件事（单一职责），容易单独测试和修改
//    - 如果以后想支持新运算符，只需要改 Lexer（加 token）和 Parser（加规则）
//    - evaluate() 的逻辑不受影响
//    - 如果以后想换一种输入格式（比如从文件读），只需要改 Lexer

#include "calculator.h"
#include "lexer.h"
#include "parser.h"
#include <cmath>        // std::isnan, std::isinf
#include <stdexcept>    // std::runtime_error

namespace calc {

double evaluate(const std::string& expression) {
    // 快速拒绝：空字符串没有意义
    if (expression.empty()) {
        throw std::runtime_error("Empty expression");
    }

    // ===== 第一阶段：词法分析 =====
    // Lexer 扫描字符串，输出 Token 列表
    // 如果遇到非法字符（如 @），这里就会抛异常
    Lexer lexer(expression);
    auto tokens = lexer.tokenize();

    // ===== 第二阶段：语法分析 =====
    // Parser 检查 Token 的排列是否符合数学语法，并构建表达式树
    // 如果语法错误（如括号不匹配、缺操作数），抛异常
    Parser parser(std::move(tokens), expression);
    auto ast = parser.parse();

    // ===== 第三阶段：求值 =====
    // 递归计算整棵表达式树的值
    // 如果出现数学错误（除零、溢出等），抛异常
    double result = ast->evaluate();

    // 最终安全检查：正常运算不会产生 NaN 或 Infinity
    // 但某些极端情况（如 std::pow 的边界行为）可能漏进来
    if (std::isnan(result)) {
        throw std::runtime_error("Result is not a number (NaN)");
    }
    if (std::isinf(result)) {
        throw std::runtime_error("Result overflow (infinity)");
    }

    return result;
}

} // namespace calc
