cmake_minimum_required(VERSION 3.16)

project(chat LANGUAGES CXX)

# 查找 Qt6 或 Qt5（根据你安装的版本）
# 如果你用的是 Qt6（推荐）：
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# 如果你用的是 Qt5，替换上面为：
# find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# 启用 AUTOMOC、AUTOUIC、AUTORCC（关键！）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件列表
file(GLOB_RECURSE SOURCES
    *.cpp
    *.h
)

# 或者显式列出（更可靠）：
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    LeftPanel.cpp
    LeftPanel.h
    RightPanel.cpp
    RightPanel.h
)

# 创建可执行文件
add_executable(chat ${SOURCES})

# 链接 Qt 库
# Qt6:
target_link_libraries(chat Qt6::Core Qt6::Widgets Qt6::Gui)

# 如果是 Qt5，用下面这行代替上面：
# target_link_libraries(chat Qt5::Core Qt5::Widgets Qt5::Gui)
