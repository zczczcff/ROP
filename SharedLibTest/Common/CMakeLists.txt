cmake_minimum_required(VERSION 3.10)

project(SharedLibTestCommon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_FILES
    Include/SharedLibTest/DynamicLoader.h
)

set(SOURCE_FILES
    Src/DynamicLoader.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:include>
)

# Windows 特定设置
if(WIN32)
    # 确保动态加载 API 可用
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601
    )
endif()
