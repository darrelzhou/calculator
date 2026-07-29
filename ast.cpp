// ============================================================================
//  ast.cpp — AST 节点的求值（evaluate）实现
// ============================================================================
//  这里实现了每种表达式树节点"怎么算出结果"。
//  求值是递归的：一个 BinaryOpNode 求值时，会先让左右子树各自求值，
//  拿到两个数值后再做运算。这就像是先算括号里的东西。

#include "ast.h"
#include <cmath>        // std::pow, std::floor, std::isnan, std::isinf
#include <limits>       // 数值极限相关（备用）
#include <sstream>      // （备用）
#include <stdexcept>    // std::runtime_error

namespace calc {

// =========================================================================
//  一元运算求值：-x
// =========================================================================
//  递归求子节点的值，然后取负。
//  支持多层负号：--3 = -(-3) = 3（因为 evaluate() 是递归的）
double UnaryOpNode::evaluate() const {
    double val = operand->evaluate();   // 先让子节点算出自己的值
    if (op == '-') {
        return -val;                    // 取负
    }
    // 如果未来支持一元 '+'，就在这里加
    return val;
}

// =========================================================================
//  二元运算求值：x + y, x - y, x * y, x / y, x ^ y
// =========================================================================
double BinaryOpNode::evaluate() const {
    // 递归求左右子树的值
    double l = left->evaluate();
    double r = right->evaluate();

    switch (op) {
        // ---- 加减乘：直接算，不会出错 ----
        case '+': return l + r;
        case '-': return l - r;
        case '*': return l * r;

        // ---- 除法：必须检查除数是否为零 ----
        case '/':
            // 浮点数不能用 == 精确判断，但判零是个合理的近似
            if (r == 0.0) {
                throw std::runtime_error("Division by zero");
            }
            return l / r;

        // ---- 乘方：用 std::pow，但要处理三种特殊情况 ----
        case '^': {
            // 情况 1：0 的负次幂 → 等同于除以零，未定义
            //   例如 0^(-2) = 1/(0^2) = 1/0 → 报错
            if (l == 0.0 && r < 0.0) {
                throw std::runtime_error(
                    "Zero raised to a negative power (division by zero)");
            }

            // 情况 2：负数的非整数次幂 → 结果会是复数
            //   例如 (-2)^0.5 = √(-2)，在实数范围内无意义
            //   std::floor(r) != r  就是判断 r 是不是整数
            if (l < 0.0 && std::floor(r) != r) {
                throw std::runtime_error(
                    "Negative base raised to a non-integer exponent "
                    "would produce a complex number");
            }

            double result = std::pow(l, r);   // 标准库的幂函数

            // 情况 3：结果太大（溢出）或算出 NaN
            //   std::pow 遇到算不了的情况可能返回 inf 或 NaN
            if (std::isinf(result)) {
                throw std::runtime_error("Numeric overflow in exponentiation");
            }
            if (std::isnan(result)) {
                throw std::runtime_error("Undefined result in exponentiation (NaN)");
            }

            return result;
        }

        // ---- 理论上不会到这里（Parser 只生成上面的运算符） ----
        default:
            throw std::runtime_error("Unknown binary operator");
    }
}

} // namespace calc
