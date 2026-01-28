cmake_minimum_required(VERSION 3.10)

# 创建独立的测试可执行文件
project(ROPTest LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加测试可执行文件
add_executable(${PROJECT_NAME} Test.cpp)

# 链接主库
target_link_libraries(${PROJECT_NAME} ROP)

# 添加测试
enable_testing()
add_test(
    NAME ${PROJECT_NAME}_test
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
)