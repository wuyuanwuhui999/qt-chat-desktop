cmake_minimum_required(VERSION 3.16)

project(chat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)  # 启用资源编译

# 查找Qt6组件 - 添加 WebSockets 模块
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network WebSockets)

if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Please install Qt6 with required modules.")
endif()

qt_standard_project_setup()

# 添加资源文件
set(QRC_FILES resources/resources.qrc)

# 添加可执行文件
add_executable(chat
    ${QRC_FILES}
    src/main.cpp
    # UI组件
    src/ui/LeftPanel.h
    src/ui/LeftPanel.cpp
    src/ui/RightPanel.h
    src/ui/RightPanel.cpp
    src/ui/WelcomeWindow.h
    src/ui/WelcomeWindow.cpp
    src/ui/LoginWindow.h
    src/ui/LoginWindow.cpp
    src/ui/HomeWindow.h
    src/ui/HomeWindow.cpp
    src/ui/DirectoryDialog.h
    src/ui/DirectoryDialog.cpp
    # 模型类
    src/models/ApiResponse.h
    src/models/ApiResponse.cpp
    src/models/User.h
    src/models/User.cpp
    # 网络类
    src/network/NetworkManager.h
    src/network/NetworkManager.cpp
    # 工具类
    src/utils/TokenManager.h
    src/utils/TokenManager.cpp
    # 主题
    src/theme/Colors.h
    src/theme/Dimens.h
    src/theme/Styles.h
    # 配置
    src/config/Constants.h
)

# 链接Qt库 - 添加 Qt6::WebSockets
target_link_libraries(chat
    PRIVATE Qt6::Core
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::Gui
    PRIVATE Qt6::Network
    PRIVATE Qt6::WebSockets
)

# 设置包含目录
target_include_directories(chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/theme
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
)

set_target_properties(chat PROPERTIES
    WIN32_EXECUTABLE TRUE
)
