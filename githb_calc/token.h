#pragma once
// ^^^^^^^^ 头文件保护：防止同一个头文件被多次 #include 导致重复定义

#include <cstddef>   // std::size_t
#include <string>

// ============================================================================
//  token.h — 词法单元（Token）的定义
// ============================================================================
//  这是整个项目中最底层的数据结构。
//  想象你在读一个数学表达式如 "3.14 + 2^3"，你需要先把每个"词"切出来：
//    "3.14" 是一个数字，"+" 是一个运算符，"^" 也是运算符……
//  切出来的每一个"词"就是一个 Token。
//
//  本文件定义两样东西：
//    1. TokenType 枚举 — 这个词是什么"类型"（数字？加号？括号？）
//    2. Token 结构体   — 这个词的完整信息（类型 + 数值 + 位置）

namespace calc {

// ---- Token 的类型标签 ----
// 用 enum class 而非普通 enum，避免名字污染（比如不会和 windows.h 冲突）
enum class TokenType {
    Number,     // 数字，如 42、3.14、.5
    Plus,       // +
    Minus,      // -
    Multiply,   // *
    Divide,     // /
    Power,      // ^  （乘方，例如 2^3 = 8）
    LParen,     // (  左圆括号
    RParen,     // )  右圆括号
    LBracket,   // [  左方括号
    RBracket,   // ]  右方括号
    LBrace,     // {  左花括号
    RBrace,     // }  右花括号
    End         // 输入结束标记（不是用户输入的，是程序自己加的）
};

// ---- 把 TokenType 转成人类能读的字符串 ----
// 例如 tokenTypeName(TokenType::Plus) 返回 "'+'"
// 这个函数只在报错时用到，告诉用户"你这里写了个什么东西"
// inline 表示函数体直接嵌入调用处，避免额外的函数调用开销
inline std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::Number:   return "number";
        case TokenType::Plus:     return "'+'";
        case TokenType::Minus:    return "'-'";
        case TokenType::Multiply: return "'*'";
        case TokenType::Divide:   return "'/'";
        case TokenType::Power:    return "'^'";
        case TokenType::LParen:   return "'('";
        case TokenType::RParen:   return "')'";
        case TokenType::LBracket: return "'['";
        case TokenType::RBracket: return "']'";
        case TokenType::LBrace:   return "'{'";
        case TokenType::RBrace:   return "'}'";
        case TokenType::End:      return "end of expression";
        default:                  return "unknown";
    }
}

// ---- 一个词法单元（Token）的完整信息 ----
// 三个字段：
//   type  — 这个词是什么类型
//   value — 如果它是数字，数值是多少（运算符和括号的 value 无意义，填 0）
//   pos   — 它在原始输入字符串中的第几个字符（从 0 开始数）
//           这个位置信息非常重要！报错时用它来画那个 ^ 箭头
struct Token {
    TokenType   type;
    double      value = 0.0;
    std::size_t pos   = 0;    // std::size_t 是无符号整数，专用于表示大小/索引
};

} // namespace calc

