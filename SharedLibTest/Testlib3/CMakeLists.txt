cmake_minimum_required(VERSION 3.10)

project(Testlib3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DTestlib3_EXPORTS)

set(HEADER_FILES
    Include/Testlib3/Testlib3Export.h
    Include/Testlib3/Test3Class.h
)

set(SOURCE_FILES
    Src/Test3Class.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/SharedLibTest/TestCore/Include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ROP
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADER_FILES}
    DESTINATION include/Testlib3
)
