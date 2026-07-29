#pragma once

#include "token.h"
#include <string>
#include <vector>

namespace calc {

// ============================================================================
//  lexer.h — 词法分析器（Lexer）的声明
// ============================================================================
//  词法分析是整个流程的第一步，它的任务纯粹而单一：
//    把用户输入的字符串 → 切成一个个 Token 的列表
//
//  举例：
//    输入: "2 + 3.14"
//    输出: [Number(2), Plus, Number(3.14), End]
//
//  Lexer 不关心语法（比如两个加号连着写是不是合法），
//  它只关心"这个字符能构成什么词"。语法检查是 Parser 的事。
//
//  为什么要把"切词"独立出来？
//    → 单一职责原则：每个类只做一件事，好写、好测、好改。
//       如果以后想支持新运算符（比如 %），只改 Lexer 就行，
//       Parser 和求值部分不受影响。

class Lexer {
public:
    // 构造函数：接收一个表达式字符串，存起来准备分析
    // explicit 禁止隐式类型转换（防止不小心把 string 变成 Lexer）
    explicit Lexer(std::string input);

    // 核心方法：对整个字符串做词法分析
    // 返回值：Token 的列表（vector<Token>）
    // 如果遇到非法字符（比如 @、#），会抛出 runtime_error，
    // 错误消息里会包含"第几列"和 ^ 箭头指示
    std::vector<Token> tokenize();

private:
    std::string input_;       // 原始输入字符串，不动它
    std::size_t pos_ = 0;     // 当前读到了第几个字符（就像光标位置）

    // ---- 逐字符读取辅助方法 ----
    // peek():  偷看当前位置的字符，但不移动光标
    // advance(): 读取当前字符，然后把光标往后移一格
    // 这两个操作用法类似"看一眼"和"吃掉"的区别
    char peek()   const;
    char advance();
    bool isAtEnd() const;     // 光标是否已经到达字符串末尾

    void skipWhitespace();    // 跳过空格、制表符、换行等无意义字符
    Token readNumber();       // 从当前位置开始读一个完整的数字（如 3.14）

    // 生成带位置信息的错误消息，像这样：
    //   Lexer error at column 5: unexpected character '@'
    //   2+3@4
    //       ^
    // [[nodiscard]] 表示返回值不应该被忽略（编译器会警告）
    [[nodiscard]] std::string makeError(const std::string& msg) const;
};

} // namespace calc
