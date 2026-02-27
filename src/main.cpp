#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // 在这里添加设置窗口标题的代码
    w.setWindowTitle("chat");  // 或者你想要的其他标题

    w.show();
    return a.exec();
}
