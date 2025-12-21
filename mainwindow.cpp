#include "mainwindow.h"
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建左右面板
    leftPanel = new LeftPanel(this);
    rightPanel = new RightPanel(this);

    // 创建水平分割器
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);

    // 设置初始比例：左 20%，右 80%
    int totalWidth = QGuiApplication::primaryScreen()->size().width();
    QList<int> sizes = { static_cast<int>(totalWidth * 0.2), static_cast<int>(totalWidth * 0.8) };
    splitter->setSizes(sizes);

    // 限制左侧面板最小/最大宽度（10% ~ 50%）
    int minWidth = static_cast<int>(totalWidth * 0.1);
    int maxWidth = static_cast<int>(totalWidth * 0.5);
    leftPanel->setMinimumWidth(minWidth);
    leftPanel->setMaximumWidth(maxWidth);

    setCentralWidget(splitter);
}
