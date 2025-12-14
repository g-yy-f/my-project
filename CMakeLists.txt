# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(my-project)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 抑制 std::result_of 的弃用警告（解决C++17警告问题）
add_compile_definitions(_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

# 创建核心库（包含 lthread.cpp 和 workqueue.cpp）
add_library(core
    src/core/lthread.cpp
    src/core/workqueue.cpp
)

# 设置库的头文件包含路径
# 关键：添加根目录，解决 #include "src/util/spinmutex.hpp" 问题
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}        # 根目录，解决头文件包含问题
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
)

# 创建可执行文件（只包含 main.cpp）
add_executable(${PROJECT_NAME}
    src/core/main.cpp
)

# 设置可执行文件的头文件包含路径
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}        # 根目录，解决头文件包含问题
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
)

# 将库链接到可执行文件
target_link_libraries(${PROJECT_NAME} core)

# 设置编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_options(core PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)