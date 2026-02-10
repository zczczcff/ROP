cmake_minimum_required(VERSION 3.10)

project(MainApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    Src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/SharedLibTest/Common/Include
    ${CMAKE_SOURCE_DIR}/SharedLibTest/TestCore/Include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ROP
    TestCore
    SharedLibTestCommon
)

# Windows 特定设置
if(WIN32)
    # 确保可以找到 DLL
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# 将依赖的 DLL 复制到运行目录
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:TestCore>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Test1lib>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Test2lib>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying required DLLs to executable directory"
    )
endif()

# 添加测试
enable_testing()
add_test(
    NAME MainApp_test
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
