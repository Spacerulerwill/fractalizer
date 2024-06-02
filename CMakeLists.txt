cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(fractalizer C)

add_executable(${PROJECT_NAME} 
    src/main.c
)

add_subdirectory(lib/raylib)
add_subdirectory(lib/raylib-nuklear)

target_include_directories(${PROJECT_NAME} 
    SYSTEM PRIVATE lib/raylib/include
    SYSTEM PRIVATE lib/raylib-nuklear/include
    PUBLIC include
    PUBLIC src
)

target_link_directories(${PROJECT_NAME}
    PRIVATE lib/raylib/src
    PRIVATE lib/raylib-nuklear/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC raylib
    PUBLIC raylib_nuklear
)

# copy our resources to executable location
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME}  PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4996 /external:W0)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -pedantic -Wcast-align -Wunused -Wpedantic -Wconversion -Wsign-conversion)
endif()