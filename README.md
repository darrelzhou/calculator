ecalc.exe 是计算器的可执行文件

# Expression Calculator

基于**表达式树 (Expression Tree)** 的数学表达式求值器，C++17 实现，工程化的模块设计。

## 特性

- **五种运算符**：`+` `-` `*` `/` `^`（`^` 为乘方，右结合）
- **三种括号**：`()` `[]` `{}` 均可用于分组，必须同类型配对
- **一元负号**：支持 `-3`、`2^-3`、`-(2+3)` 等
- **完整错误提示**：词法/语法/数学错误均带列号定位
- **I/O 与核心逻辑分离**：核心函数 `calc::evaluate(string) → double` 零 I/O 依赖

## 项目结构

```
githb_calc/
├── token.h          # Token 类型定义
├── lexer.h/cpp      # 词法分析器（字符串 → Token 序列）
├── ast.h/cpp        # AST 节点定义与求值
├── parser.h/cpp     # 递归下降语法分析器（Token → AST）
├── calculator.h/cpp # 核心 API（纯函数）
├── calc.cpp         # I/O 层（入口 main）
├── CMakeLists.txt   # CMake 构建
└── README.md
```

### 模块职责

| 模块 | 职责 |
|------|------|
| `Lexer` | 词法分析：扫描字符，输出 Token 流 |
| `Parser` | 语法分析：递归下降构建表达式树 |
| `ASTNode` | 表达式树节点：`NumberNode` / `UnaryOpNode` / `BinaryOpNode` |
| `evaluate()` | 核心纯函数：串联词法→语法→求值，对外唯一接口 |

## 构建与运行

### 前置条件

- C++17 编译器（g++ ≥ 7, clang++ ≥ 5, MSVC ≥ 2017）
- CMake ≥ 3.14

### 使用 CMake

```bash
cd githb_calc
cmake -B build
cmake --build build
./build/ecalc        # Linux / macOS
build\Debug\ecalc.exe  # Windows
```

### 直接使用 g++

```bash
cd githb_calc
g++ -std=c++17 -Wall -static -o ecalc calc.cpp calculator.cpp lexer.cpp parser.cpp ast.cpp
./ecalc
```

## 使用示例

```
> 2+3*4
= 14

> (1+2)^3
= 27

> 2^-3
= 0.125

> {2+[3*(4+5)]}
= 29

> -2^2
= -4

> 10/0
Error: Division by zero
```

## 运算符优先级

| 优先级 | 运算符 | 结合性 |
|--------|--------|--------|
| 1 (最高) | `()` `[]` `{}` | — |
| 2 | `-` (一元) | 右 |
| 3 | `^` | 右 |
| 4 | `*` `/` | 左 |
| 5 (最低) | `+` `-` | 左 |

## 错误处理

所有错误以 `std::runtime_error` 抛出，消息含列号定位，例如：

```
Error: Parse error at column 5: unexpected token ')', expected a number or '('
2+3*)
    ^
```

## 许可

MIT
