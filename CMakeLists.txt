cmake_minimum_required(VERSION 3.16)

project(chat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)  # 启用自动moc处理

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# 检查是否找到Widgets
if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Please install Qt6 with Widgets module.")
endif()

qt_standard_project_setup()

# 添加可执行文件
add_executable(appchat3
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/ui/LeftPanel.h
    src/ui/LeftPanel.cpp
    src/ui/RightPanel.h
    src/ui/RightPanel.cpp
)

# 链接Qt库
target_link_libraries(appchat3
    PRIVATE Qt6::Core
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::Gui
)

# 设置包含目录（确保能找到所有头文件）
target_include_directories(appchat3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

set_target_properties(appchat3 PROPERTIES
    WIN32_EXECUTABLE TRUE
)
