/// 内嵌单元测试 —— 直接测试核心计算逻辑
#include "calculator.h"
#include <iostream>
#include <cmath>
#include <string>

static int passed = 0, failed = 0;

static void test(const std::string& expr, double expected) {
    try {
        double result = calc::evaluate(expr);
        if (std::abs(result - expected) < 1e-9) {
            ++passed;
            std::cout << "[PASS] " << expr << " = " << result << "\n";
        } else {
            ++failed;
            std::cout << "[FAIL] " << expr << " : expected " << expected << ", got " << result << "\n";
        }
    } catch (const std::runtime_error& e) {
        ++failed;
        std::cout << "[FAIL] " << expr << " : threw \"" << e.what() << "\"\n";
    }
}

static void testError(const std::string& expr) {
    try {
        double result = calc::evaluate(expr);
        ++failed;
        std::cout << "[FAIL] " << expr << " : expected error, got " << result << "\n";
    } catch (const std::runtime_error&) {
        ++passed;
        std::cout << "[PASS] " << expr << " -> error (expected)\n";
    }
}

int main() {
    std::cout << "=== Expression Calculator Tests ===\n\n";

    // 基础四则运算
    test("2+3", 5);
    test("10-7", 3);
    test("4*5", 20);
    test("20/4", 5);
    test("2+3*4", 14);         // 优先级：* 高于 +
    test("10-6/2", 7);         // 优先级：/ 高于 -
    test("2*3+4*5", 26);

    // 乘方
    test("2^3", 8);
    test("2^10", 1024);
    test("2^3^2", 512);        // 右结合：2^(3^2) = 2^9 = 512
    test("2^(3^2)", 512);
    test("(2^3)^2", 64);       // (2^3)^2 = 8^2 = 64

    // 一元负号
    test("-3", -3);
    test("--3", 3);
    test("-2^2", -4);          // -(2^2) = -4，乘方优先于负号
    test("(-2)^2", 4);
    test("2^-3", 0.125);       // 2^(-3) = 1/8
    test("2*-3", -6);
    test("-3+5", 2);
    test("2--3", 5);           // 2 - (-3) = 5

    // 括号
    test("(2+3)*4", 20);
    test("[2+3]*4", 20);
    test("{2+3}*4", 20);
    test("{[2+3]*4}", 20);
    test("2*(3+4*(5-2))", 30);

    // 小数
    test("3.14", 3.14);
    test(".5+.5", 1.0);
    test("2.5*4", 10.0);

    // 边界
    test("0^5", 0);
    test("5^0", 1);
    test("0^0", 1);            // std::pow(0,0) = 1

    // 错误情况
    std::cout << "\n--- Error cases ---\n";
    testError("10/0");         // 除零
    testError("0^-1");         // 0 的负次幂
    testError("(-2)^0.5");     // 负底数非整数次幂
    testError("");             // 空表达式
    testError("(2+3");         // 括号不匹配
    testError("2+3)");         // 多余右括号
    testError("2++3");         // 连续运算符（无操作数）
    testError("2 3");          // 缺运算符
    testError("2@3");          // 非法字符

    std::cout << "\n=== Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed > 0 ? 1 : 0;
}
