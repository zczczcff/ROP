cmake_minimum_required(VERSION 3.10)

project(SharedLibTest VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加 Common 库（静态库）
add_subdirectory(Common)

# 添加动态库
add_subdirectory(TestCore)
add_subdirectory(Test1lib)
add_subdirectory(Test2lib)
add_subdirectory(Testlib3)

# 添加主程序
add_subdirectory(MainApp)

# 打印配置信息
message(STATUS "SharedLibTest Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
