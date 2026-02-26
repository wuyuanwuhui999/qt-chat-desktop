# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(chat LANGUAGES CXX)

# 查找 Qt6 或 Qt5
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# 启用 AUTOMOC、AUTOUIC、AUTORCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 方式1：使用 GLOB_RECURSE 递归查找所有源文件（简单但注意新增文件需要重新运行CMake）
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/*.ui"
)

# 方式2：显式列出所有源文件（更可靠，推荐）
set(SOURCES
    # 根目录源文件
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui

    # ui 目录
    src/ui/leftpanel.cpp
    src/ui/leftpanel.h
    src/ui/rightpanel.cpp
    src/ui/rightpanel.h

    # 其他目录（目前暂无文件，但添加占位以便显示）
    # 当这些目录有文件时，在这里添加
    # src/config/...
    # src/model/...
    # src/network/...
    # src/theme/...
    # src/utils/...
)

# 创建可执行文件
add_executable(chat ${SOURCES})

# 链接 Qt 库
target_link_libraries(chat Qt6::Core Qt6::Widgets Qt6::Gui)

# 为了让Qt Creator显示空目录，可以添加伪目标文件（可选）
# 如果不希望显示空目录，可以忽略这一步
