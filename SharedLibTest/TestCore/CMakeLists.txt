cmake_minimum_required(VERSION 3.10)

project(TestCore LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义导出宏
add_definitions(-DTestCore_EXPORTS)

# 收集头文件
set(HEADER_FILES
    Include/TestCore/TestCoreExport.h
    Include/TestCore/CorePropertyType.h
    Include/TestCore/TestCore.h
)

# 收集源文件
set(SOURCE_FILES
    Src/TestCore.cpp
)

# 创建共享库（动态库）
add_library(${PROJECT_NAME} SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_SOURCE_DIR}/Include
)

# 链接 ROP 库
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ROP
)

# Windows 特定设置
if(WIN32)
    # 确保正确导出符号
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
endif()

# 安装规则（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADER_FILES}
    DESTINATION include/TestCore
)
