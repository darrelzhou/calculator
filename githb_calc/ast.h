#pragma once

#include <memory>   // std::unique_ptr（独占所有权的智能指针）

namespace calc {

// ============================================================================
//  ast.h — 抽象语法树（AST）节点的定义
// ============================================================================
//  这是"表达式树"的核心数据结构。
//
//  什么叫做"表达式树"？
//  数学表达式有结构。比如 "2 + 3 * 4" 并不是从左到右算的，
//  而是先算 3*4=12，再算 2+12=14。这个"先乘除后加减"的结构，
//  用一棵树来表示最自然：
//
//        (+)
//        / \
//       2  (*)
//          / \
//         3   4
//
//  每个节点都是 ASTNode 的子类：
//    - NumberNode   : 叶子节点，存一个数字
//    - UnaryOpNode  : 一元运算，如 -3（一个子节点）
//    - BinaryOpNode : 二元运算，如 2+3（左右两个子节点）
//
//  为什么要用继承（面向对象）？
//    不同的节点有不同的 evaluate() 行为：
//      数字节点直接返回数值；
//      运算符节点先递归求左右子树的值，再做运算。
//    用虚函数 + 继承，Parser 不需要知道"这个节点具体是什么类型"，
//    只需要调用 evaluate()，多态自动分发到正确的实现。
//    这叫做"里氏替换原则"——子类可以无缝替换父类。

// ---- 基类：所有 AST 节点的公共接口 ----
struct ASTNode {
    // 虚析构函数：确保通过基类指针删除子类对象时，
    // 子类的析构函数也会被正确调用（防止内存泄漏）
    virtual ~ASTNode() = default;

    // 纯虚函数：每个子类必须实现自己的 evaluate()
    // = 0 表示"这个函数没有默认实现，子类必须写"
    // [[nodiscard]]：调用者不应该忽略返回值
    [[nodiscard]] virtual double evaluate() const = 0;
};

// ---- 数字节点（叶子节点） ----
// 例如表达式 "42" 就只生成一个 NumberNode(42)
struct NumberNode : ASTNode {
    double value;

    explicit NumberNode(double v) : value(v) {}

    // 数字的求值最简单：直接返回自己的数值
    double evaluate() const override { return value; }
};

// ---- 一元运算节点 ----
// 例如 "-3" 生成 UnaryOpNode('-', NumberNode(3))
// 一元运算只有一个子节点（operand）
// 目前只支持负号 '-'，但架构上预留了扩展空间
struct UnaryOpNode : ASTNode {
    char op;                              // 运算符，目前只有 '-'
    std::unique_ptr<ASTNode> operand;     // 唯一的子节点
    // unique_ptr 表示这个节点"独占"子节点的所有权，
    // 当这个节点被销毁时，子节点也会自动销毁，不会内存泄漏

    UnaryOpNode(char o, std::unique_ptr<ASTNode> operand_)
        : op(o), operand(std::move(operand_)) {}
    // std::move 转移所有权：operand_ 把它的子节点"送"给 this->operand

    double evaluate() const override;
};

// ---- 二元运算节点 ----
// 例如 "2 + 3" 生成 BinaryOpNode('+', NumberNode(2), NumberNode(3))
// 二元运算有左右两个子节点
// 支持的运算：+, -, *, /, ^
struct BinaryOpNode : ASTNode {
    char op;                              // 运算符：'+' '-' '*' '/' '^'
    std::unique_ptr<ASTNode> left;        // 左子树
    std::unique_ptr<ASTNode> right;       // 右子树

    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}

    double evaluate() const override;
};

} // namespace calc
