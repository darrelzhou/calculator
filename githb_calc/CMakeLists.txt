cmake_minimum_required(VERSION 3.14)
project(ExpressionCalculator VERSION 1.0.0 LANGUAGES CXX)

# ---- C++ 标准 ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- 可执行文件 ----
add_executable(ecalc
    calc.cpp          # I/O 层（入口）
    calculator.cpp    # 核心 API
    lexer.cpp         # 词法分析
    parser.cpp        # 语法分析
    ast.cpp           # AST 节点 + 求值
)

# 头文件位于当前目录
target_include_directories(calc PRIVATE ${CMAKE_SOURCE_DIR})

# ---- 编译警告 ----
if(MSVC)
    target_compile_options(calc PRIVATE /W4)
else()
    target_compile_options(calc PRIVATE -Wall -Wextra -Wpedantic)
endif()
