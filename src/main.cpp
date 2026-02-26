#include "mainwindow.h"
#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    // 设置窗口为全屏（屏幕大小）
    QScreen *screen = QGuiApplication::primaryScreen();
    w.resize(screen->size());
    w.setWindowTitle("chat");
    w.show();

    return app.exec();
}
