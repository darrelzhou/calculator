#include "calculator.h"
#include "lexer.h"
#include "parser.h"
#include <cmath>
#include <stdexcept>

namespace calc {

double evaluate(const std::string& expression) {
    if (expression.empty()) {
        throw std::runtime_error("Empty expression");
    }

    // 第一阶段：词法分析
    Lexer lexer(expression);
    auto tokens = lexer.tokenize();

    // 第二阶段：语法分析 → 构建表达式树
    Parser parser(std::move(tokens), expression);
    auto ast = parser.parse();

    // 第三阶段：对表达式树求值
    double result = ast->evaluate();

    // 最终结果校验
    if (std::isnan(result)) {
        throw std::runtime_error("Result is not a number (NaN)");
    }
    if (std::isinf(result)) {
        throw std::runtime_error("Result overflow (infinity)");
    }

    return result;
}

} // namespace calc
