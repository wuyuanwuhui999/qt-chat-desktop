#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>

// 包含子控件头文件
#include "LeftPanel.h"
#include "RightPanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT  // 👈 添加这个！否则无法正确链接！

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QSplitter *splitter;
    LeftPanel *leftPanel;
    RightPanel *rightPanel;
};

#endif // MAINWINDOW_H
