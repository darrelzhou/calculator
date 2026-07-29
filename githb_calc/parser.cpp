// ============================================================================
//  parser.cpp — 语法分析器（递归下降解析）的实现
// ============================================================================
//  这是项目中最核心、最复杂的模块。
//  工作原理：把 Token 列表当作一个"流"，从头到尾读过去，
//  一边读一边按照数学运算的优先级构建表达式树。
//
//  【核心概念：递归下降】
//  每个"优先级层级"对应一个函数，高层函数调用低层函数。
//  比如 expression() 处理加减 → 它调用 term() 处理乘除 →
//  term() 调用 unary() 处理负号 → unary() 调用 power() 处理乘方 →
//  power() 调用 primary() 处理数字和括号。
//
//  【结合性】
//  左结合（加减乘除）：while 循环 ← 因为 1-2-3 要算成 (1-2)-3
//  右结合（乘方）：    递归     ← 因为 2^3^4 要算成 2^(3^4)
//
//  【为什么 powerRight 要单独设计】
//  2^-3 这个表达式里，^ 的右边是 -3（包含一元负号）。
//  如果 power() 右侧也像左侧一样只调用 primary()，就读不到 -。
//  所以设计 powerRight()，它的"左操作数"走 unary() 路径，能处理负号。
//  但主体 power() 的左侧仍然走 primary()，保证 -2^2 = -(2^2) 的正确优先级。

#include "parser.h"
#include <sstream>
#include <stdexcept>

namespace calc {

// ---- 构造函数 ---------------------------------------------------------------
Parser::Parser(std::vector<Token> tokens, std::string originalExpr)
    : tokens_(std::move(tokens)), originalExpr_(std::move(originalExpr)) {}

// =========================================================================
//  Token 流的基本操作
// =========================================================================
//  把 Token 列表当成一个只能往前读的流，提供 peek（看）、advance（取）、
//  match（如果符合预期就取走）等操作。这套接口让解析代码读起来像自然语言。

const Token& Parser::peek() const {
    return tokens_[current_];          // 看当前 Token，不移动
}

const Token& Parser::advance() {
    return tokens_[current_++];        // 取走当前 Token，下标前移
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];      // 看上一个被取走的 Token
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::End;   // 读到末尾标记了
}

bool Parser::check(TokenType type) const {
    return peek().type == type;        // 当前 Token 是指定类型吗？
}

// match: "如果当前 Token 是列表中任意一种，就吃掉它并返回 true；
//         否则什么也不做，返回 false"
// 这是递归下降解析最常用的操作。
// initializer_list 允许这样调用：match({Plus, Minus})
bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto t : types) {
        if (check(t)) {
            advance();                 // 匹配成功，吃掉
            return true;
        }
    }
    return false;                      // 一个都没匹配上
}

// consume: 跟 match 类似，但匹配不上时不是返回 false，而是直接报错
// 用于"这里必须是某个东西"的场合
const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error(makeError(message));
}

// =========================================================================
//  错误消息
// =========================================================================
//  报错时会画出原始输入和 ^ 箭头，像这样：
//    Parse error at column 5: unexpected token ')'
//    2+3*)
//        ^

std::string Parser::makeError(const std::string& msg) const {
    return makeErrorAt(peek(), msg);
}

std::string Parser::makeErrorAt(const Token& token, const std::string& msg) const {
    std::ostringstream oss;
    oss << "Parse error at column " << (token.pos + 1) << ": " << msg << "\n";
    oss << originalExpr_ << "\n";
    if (token.pos < originalExpr_.size()) {
        oss << std::string(token.pos, ' ') << "^\n";
    }
    return oss.str();
}

// =========================================================================
//  括号匹配
// =========================================================================
//  三种括号 (), [], {} 都可以用来分组，但必须配对：
//  ( 只能和 ) 配对，不能和 ] 或 } 配对。

TokenType Parser::getClosingBracket(TokenType open) {
    switch (open) {
        case TokenType::LParen:   return TokenType::RParen;
        case TokenType::LBracket: return TokenType::RBracket;
        case TokenType::LBrace:   return TokenType::RBrace;
        default:                  return TokenType::End;   // 不会发生
    }
}

std::string Parser::bracketName(TokenType bracket) {
    switch (bracket) {
        case TokenType::LParen:   return "'('";
        case TokenType::RParen:   return "')'";
        case TokenType::LBracket: return "'['";
        case TokenType::RBracket: return "']'";
        case TokenType::LBrace:   return "'{'";
        case TokenType::RBrace:   return "'}'";
        default:                  return "unknown bracket";
    }
}

// =========================================================================
//  解析入口
// =========================================================================
//  整个解析流程的起点：解析整个表达式，然后检查有没有多余的垃圾 token。
std::unique_ptr<ASTNode> Parser::parse() {
    auto ast = expression();   // 从最低优先级的加减开始

    // 合法表达式解析完后，应该正好遇到 End 标记
    // 如果后面还有东西（比如 "2+3 5" 中多余的 5），报错
    if (!isAtEnd()) {
        throw std::runtime_error(
            makeErrorAt(peek(),
                "unexpected token " + tokenTypeName(peek().type) +
                " after complete expression"));
    }
    return ast;
}

// =========================================================================
//  Expression := Term { ('+' | '-') Term }
//  处理加减法，优先级最低（最外层）
// =========================================================================
//  例子："1 + 2 - 3"
//    left = term() → 得到 1
//    while 遇到 '+': right = term() → 得到 2, left = 1+2
//    while 遇到 '-': right = term() → 得到 3, left = (1+2)-3
//  注意 while 而非递归 → 左结合
std::unique_ptr<ASTNode> Parser::expression() {
    auto left = term();   // 先解析左边的乘除表达式

    // 只要后面还有 + 或 -，就继续"粘"到右边
    while (match({TokenType::Plus, TokenType::Minus})) {
        char op = (previous().type == TokenType::Plus) ? '+' : '-';
        auto right = term();   // 右边的操作数
        // 新建二元节点，把之前的结果当成左子树
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// =========================================================================
//  Term := Unary { ('*' | '/') Unary }
//  处理乘除法，优先级高于加减
// =========================================================================
//  结构跟 expression() 完全一样，只是运算符不同。
std::unique_ptr<ASTNode> Parser::term() {
    auto left = unary();   // 先解析左边（可能带负号）

    while (match({TokenType::Multiply, TokenType::Divide})) {
        char op = (previous().type == TokenType::Multiply) ? '*' : '/';
        auto right = unary();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// =========================================================================
//  Unary := [ '-' ] Power
//  处理一元负号，优先级高于乘除和乘方
// =========================================================================
//  关键设计点：一元负号在这里处理，在 power() 之上。
//  所以 -2^2 = -(2^2) = -4（^ 先算，- 后算，符合数学惯例）。
//
//  递归调用自己的原因：支持 ---3 = -(-(-3)) = -3
std::unique_ptr<ASTNode> Parser::unary() {
    if (match({TokenType::Minus})) {
        auto operand = unary();   // 递归！处理多重负号
        return std::make_unique<UnaryOpNode>('-', std::move(operand));
    }
    // 没有负号，进入下一层（乘方）
    return power();
}

// =========================================================================
//  Power := Primary [ '^' PowerRight ]
//  处理乘方，优先级高于一元负号、乘除、加减
//  右结合！通过 powerRight() 递归实现
// =========================================================================
//  例子 "2^3^2"：
//    left = 2
//    遇到 ^：right = powerRight()
//      powerRight: left = unary() = 3
//      遇到 ^：right = powerRight()
//        powerRight: left = unary() = 2
//      返回 3^2 = 9
//    返回 2^9 = 512  ← 右结合！
//
//  左侧用 primary() 而非 unary()：
//    保证 ^ 比一元负号优先级高（-2^2 中 ^ 先结合 2 和 2）
std::unique_ptr<ASTNode> Parser::power() {
    auto left = primary();   // 底数：数字或括号表达式

    if (match({TokenType::Power})) {
        // 指数：走 powerRight，能处理 -3 这种带负号的指数
        auto right = powerRight();
        left = std::make_unique<BinaryOpNode>('^', std::move(left), std::move(right));
    }
    return left;
}

// =========================================================================
//  PowerRight := Unary [ '^' PowerRight ]
//  跟 power() 一样，但"底数"走 unary() → 能处理一元负号
// =========================================================================
//  这是为了支持 2^-3 这种表达式：
//    2 的指数部分是 -3，需要 unary() 来处理负号。
//  如果直接用 power()（它调用 primary()），遇到 -3 就会报错。
//
//  递归结构维持了右结合性。
std::unique_ptr<ASTNode> Parser::powerRight() {
    auto left = unary();   // ← 与 power() 的唯一区别：这里用 unary()

    if (match({TokenType::Power})) {
        auto right = powerRight();   // 递归 → 右结合
        left = std::make_unique<BinaryOpNode>('^', std::move(left), std::move(right));
    }
    return left;
}

// =========================================================================
//  Primary := Number
//           | '(' Expression ')'
//           | '[' Expression ']'
//           | '{' Expression '}'
//  最底层：原子表达式
// =========================================================================
std::unique_ptr<ASTNode> Parser::primary() {
    // ---- 情况 1：数字 ----
    if (match({TokenType::Number})) {
        return std::make_unique<NumberNode>(previous().value);
    }

    // ---- 情况 2：括号分组 ----
    // 三种括号 (, [, { 都可以开始一个子表达式
    if (match({TokenType::LParen, TokenType::LBracket, TokenType::LBrace})) {
        TokenType openType      = previous().type;             // 记住是哪种左括号
        TokenType expectedClose = getClosingBracket(openType); // 对应的右括号

        // 递归解析括号里的内容（从最外层的 expression 重新开始）
        auto inner = expression();

        // 必须用同类型的右括号闭合，否则报错
        if (!match({expectedClose})) {
            if (isAtEnd()) {
                // 表达式结束了还没找到右括号
                throw std::runtime_error(
                    makeErrorAt(previous(),
                        "expected " + bracketName(expectedClose) +
                        " to match " + bracketName(openType) +
                        ", but reached end of expression"));
            } else {
                // 找到了括号，但是类型不对
                throw std::runtime_error(
                    makeErrorAt(peek(),
                        "expected " + bracketName(expectedClose) +
                        " to match " + bracketName(openType) +
                        ", but found " + tokenTypeName(peek().type)));
            }
        }

        return inner;   // 括号里的表达式就是结果
    }

    // ---- 情况 3：遇到了意料之外的 Token ----
    if (isAtEnd()) {
        throw std::runtime_error(
            makeError("unexpected end of expression, expected a number or '('"));
    }

    throw std::runtime_error(
        makeError("unexpected token " + tokenTypeName(peek().type) +
                  ", expected a number or '('"));
}

} // namespace calc
