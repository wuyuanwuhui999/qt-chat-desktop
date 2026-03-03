#include "HomeWindow.h"
#include <QGuiApplication>
#include <QScreen>  // 添加这个头文件
#include "utils/TokenManager.h"
#include "theme/Colors.h"

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent) {
    
    setWindowTitle("Chat - 主界面");
    setStyleSheet(QString("background-color: %1;").arg(Colors::PAGE_BACKGROUND_COLOR.name()));
    
    // 创建左右面板
    leftPanel = new LeftPanel(this);
    rightPanel = new RightPanel(this);
    
    // 创建水平分割器
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    
    // 设置初始比例：左 20%，右 80%
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        int totalWidth = screen->size().width();
        QList<int> sizes = { static_cast<int>(totalWidth * 0.2), static_cast<int>(totalWidth * 0.8) };
        splitter->setSizes(sizes);
        
        // 限制左侧面板最小/最大宽度（10% ~ 50%）
        int minWidth = static_cast<int>(totalWidth * 0.1);
        int maxWidth = static_cast<int>(totalWidth * 0.5);
        leftPanel->setMinimumWidth(minWidth);
        leftPanel->setMaximumWidth(maxWidth);
    }
    
    setCentralWidget(splitter);
    
    // 最大化显示
    showMaximized();
}