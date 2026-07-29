// ============================================================================
//  lexer.cpp — 词法分析器的实现
// ============================================================================
//  这里实现了 Lexer 类的所有方法。
//  核心思路：像人读句子一样，一个字符一个字符地扫过去，
//  遇到数字就"拼"出一个完整的数，遇到符号就识别成对应的运算符或括号。

#include "lexer.h"
#include <cctype>      // std::isdigit, std::isspace
#include <sstream>     // std::ostringstream（拼错误消息用）
#include <stdexcept>   // std::runtime_error

namespace calc {

// ---- 构造函数：把用户输入的字符串存起来 ----
// std::move 避免拷贝：input_ 直接"接管"了 input 的内存，效率更高
Lexer::Lexer(std::string input) : input_(std::move(input)) {}

// =========================================================================
//  内部辅助方法
// =========================================================================

// 偷看当前字符，但不往前走
// 如果已经读完了，返回 '\0'（空字符，C 风格字符串的结束标记）
char Lexer::peek() const {
    return isAtEnd() ? '\0' : input_[pos_];
}

// 读取当前字符，然后把光标 pos_ 往后移一格
// 返回的是被读走的那个字符
char Lexer::advance() {
    return input_[pos_++];   // 先取 input_[pos_]，再 pos_ += 1
}

bool Lexer::isAtEnd() const {
    return pos_ >= input_.size();
}

// 跳过所有空白字符（空格、Tab、换行等）
// static_cast<unsigned char> 是为了安全处理非 ASCII 字符
void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek()))) {
        advance();
    }
}

// 拼一个人类可读的错误消息，带 ^ 箭头指向出错的位置
// 例如输入 "2@3" 时：
//   Lexer error at column 2: unexpected character '@'
//   2@3
//    ^
std::string Lexer::makeError(const std::string& msg) const {
    std::ostringstream oss;                      // 像 cout 但是往字符串里写
    oss << "Lexer error at column " << (pos_ + 1)  // 列号从 1 开始，对人类更友好
        << ": " << msg << "\n";
    oss << input_ << "\n";                       // 打印原始输入
    if (pos_ < input_.size())
        oss << std::string(pos_, ' ') << "^\n";  // 在对应位置画 ^
    return oss.str();                            // 把拼好的字符串拿出来
}

// =========================================================================
//  读取一个数字
// =========================================================================
//  数字的格式可能是：42、3.14、.5、2.
//  算法：只要当前字符是数字或小数点，就一直往后读，
//        但不能出现两个小数点（3.14.15 是非法的）。
//
//  注意：单独的 "." 已经在 tokenize() 里拦截了，这里是防御性检查。
Token Lexer::readNumber() {
    std::size_t start = pos_;    // 记住数字的起始位置
    bool hasDecimal = false;     // 是否已经读过小数点了（防止出现两个点）

    // 循环读取：数字继续读，第一个小数点也继续读，其他情况停止
    while (!isAtEnd()) {
        char c = peek();
        if (std::isdigit(static_cast<unsigned char>(c))) {
            advance();           // 数字，吃掉
        } else if (c == '.' && !hasDecimal) {
            hasDecimal = true;   // 第一个小数点，吃掉并标记
            advance();
        } else {
            break;               // 不是数字也不是第一个小数点 → 数字结束
        }
    }

    // 截取出数字部分的子串，例如从 "3.14+2" 中截出 "3.14"
    std::string numStr = input_.substr(start, pos_ - start);

    // 防御：如果只读到一个孤立的 "."，报错
    if (numStr == ".") {
        throw std::runtime_error(makeError("isolated '.' is not a valid number"));
    }

    // std::stod = string to double，把 "3.14" 转成 3.14
    double value = std::stod(numStr);
    return Token{TokenType::Number, value, start};
}

// =========================================================================
//  主入口：对整个输入做完整的词法分析
// =========================================================================
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;   // 结果列表，逐个往里放 Token

    while (!isAtEnd()) {
        skipWhitespace();        // 先跳过空格
        if (isAtEnd()) break;    // 跳完空格发现结束了，就退出

        std::size_t start = pos_;  // 当前 token 在原始输入中的位置
        char c = peek();

        // ---- 判断当前字符属于哪种 token ----

        // 情况 1：数字开头（包括 .5 这种小数点开头的）
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            // 如果以 '.' 开头，下一个字符必须是数字（否则就是孤立的点）
            if (c == '.') {
                if (pos_ + 1 >= input_.size() ||
                    !std::isdigit(static_cast<unsigned char>(input_[pos_ + 1]))) {
                    throw std::runtime_error(
                        makeError("unexpected '.' — a leading dot must be followed by a digit"));
                }
            }
            tokens.push_back(readNumber());   // 读完整数字，加入列表

        // 情况 2：运算符或括号
        } else {
            advance();   // 吃掉这个字符
            TokenType type;
            switch (c) {
                case '+': type = TokenType::Plus;      break;
                case '-': type = TokenType::Minus;     break;
                case '*': type = TokenType::Multiply;  break;
                case '/': type = TokenType::Divide;    break;
                case '^': type = TokenType::Power;     break;
                case '(': type = TokenType::LParen;    break;
                case ')': type = TokenType::RParen;    break;
                case '[': type = TokenType::LBracket;  break;
                case ']': type = TokenType::RBracket;  break;
                case '{': type = TokenType::LBrace;    break;
                case '}': type = TokenType::RBrace;    break;
                default:
                    // 任何不在上面列表中的字符都是非法的（如 @ # $ 等）
                    throw std::runtime_error(
                        makeError("unexpected character '" + std::string(1, c) + "'"));
            }
            tokens.push_back(Token{type, 0.0, start});  // 运算符没有数值，填 0.0
        }
    }

    // 最后加一个 End 标记，告诉 Parser "输入到这里就没了"
    // 这样 Parser 遍历 Token 列表时就能知道什么时候结束
    tokens.push_back(Token{TokenType::End, 0.0, input_.size()});
    return tokens;
}

} // namespace calc
