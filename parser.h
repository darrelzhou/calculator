#pragma once

#include "token.h"
#include "ast.h"
#include <memory>
#include <string>
#include <vector>

namespace calc {

// ============================================================================
//  parser.h — 语法分析器（Parser）的声明
// ============================================================================
//  Parser 是整个项目中最复杂的模块。它的任务：
//    把 Lexer 产出的 Token 列表 → 构建成一棵 AST 表达式树
//
//  核心技术：递归下降解析（Recursive Descent Parsing）
//    这是一种"把语法规则直接翻译成函数调用"的方法。
//    每个语法规则（Expression、Term、Power……）对应一个成员函数，
//    高优先级的规则调用低优先级的规则，形成嵌套调用。
//
//  文法（即语法规则，从上到下优先级越来越高）：
//
//    Expression := Term { ('+' | '-') Term }
//        加减法：1+2-3  被解析成 (1+2)-3（左结合）
//        { ... } 表示里面的东西可以重复 0 次或多次
//
//    Term := Unary { ('*' | '/') Unary }
//        乘除法：2*3/4  被解析成 (2*3)/4（左结合）
//
//    Unary := [ '-' ] Power
//        一元负号：[ ] 表示可选（0 次或 1 次）
//        注意：Unary 在 Power 之上，意味着 ^ 比 - 优先级高
//        所以 -2^2 = -(2^2) = -4（数学上正确的优先级）
//
//    Power := Primary [ '^' PowerRight ]
//        乘方：2^3^4  被解析成 2^(3^4)（右结合！）
//        Power 的左侧是 Primary（纯数字/括号），
//        右侧是 PowerRight（允许一元负号如 2^-3）
//
//    PowerRight := Unary [ '^' PowerRight ]
//        跟 Power 结构一样，只是左侧经过 Unary（能处理 -）
//        为什么要单独设计？因为 2^-3 中，指数 -3 需要先被 Unary 处理，
//        但又不能影响 Power 的主体结构（否则会破坏 -2^2 的优先级）
//
//    Primary := Number | '(' Expression ')' | '[' Expression ']' | '{' Expression '}'
//        最底层的"原子"：要么是数字，要么是括号包起来的子表达式
//
//  为什么用递归下降而不是其他方法（如栈、状态机）？
//    递归下降最直观：代码结构直接对应数学运算的优先级结构，
//    容易理解、容易调试、容易扩展（加新运算符只加一个函数）。
//
//  关于左结合与右结合：
//    加减乘除是左结合：1-2-3 = (1-2)-3 = -4
//    乘方是右结合：  2^3^4 = 2^(3^4) = 2^81
//    在代码中，左结合用 while 循环实现，右结合用递归实现。

class Parser {
public:
    // 构造函数：接收 Token 列表 + 原始字符串（报错时画箭头用）
    Parser(std::vector<Token> tokens, std::string originalExpr);

    // 唯一的公开方法：解析并返回 AST 的根节点
    [[nodiscard]] std::unique_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens_;     // Token 列表（从 Lexer 来）
    std::string        originalExpr_; // 用户原始输入（报错时显示）
    std::size_t        current_ = 0;  // 当前正在看第几个 Token

    // ========== Token 流的"读"操作 ==========
    // 这些方法模拟一个"只读的流"，只能往前看或往前走，不能后退
    const Token& peek()     const;   // 看当前 Token，不消耗
    const Token& advance();          // 拿走当前 Token，前进到下一个
    const Token& previous() const;   // 看上一个被拿走的 Token
    bool isAtEnd()          const;   // 是不是已经读到末尾 End 了
    bool check(TokenType type) const; // 当前 Token 是不是某个类型
    bool match(std::initializer_list<TokenType> types); // "如果当前是这些类型之一，就吃掉它"
    const Token& consume(TokenType type, const std::string& message); // 必须吃到一个指定类型，否则报错

    // ========== 错误消息 ==========
    [[nodiscard]] std::string makeError(const std::string& msg) const;
    [[nodiscard]] std::string makeErrorAt(const Token& token, const std::string& msg) const;

    // ========== 递归下降解析函数（每个对应一条语法规则） ==========
    std::unique_ptr<ASTNode> expression();    // 加减
    std::unique_ptr<ASTNode> term();          // 乘除
    std::unique_ptr<ASTNode> unary();         // 一元负号
    std::unique_ptr<ASTNode> power();         // 乘方（主体）
    std::unique_ptr<ASTNode> powerRight();    // 乘方（右侧，支持一元负号）
    std::unique_ptr<ASTNode> primary();       // 原子：数字 / 括号

    // ========== 括号匹配 ==========
    static TokenType   getClosingBracket(TokenType open);
    static std::string bracketName(TokenType bracket);
};

} // namespace calc
