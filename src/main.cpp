#include <QApplication>
#include <QStackedWidget>
#include <QScreen>
#include "ui/WelcomeWindow.h"
#include "ui/LoginWindow.h"
#include "ui/HomeWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("Chat");
    a.setOrganizationName("YourCompany");
    
    // 创建堆叠窗口
    QStackedWidget stackedWidget;
    
    // 创建各个窗口
    WelcomeWindow* welcomeWindow = new WelcomeWindow;
    LoginWindow* loginWindow = new LoginWindow;
    HomeWindow* homeWindow = new HomeWindow;
    
    // 添加到堆叠窗口
    int welcomeIndex = stackedWidget.addWidget(welcomeWindow);
    int loginIndex = stackedWidget.addWidget(loginWindow);
    int homeIndex = stackedWidget.addWidget(homeWindow);
    
    // 连接信号
    QObject::connect(welcomeWindow, &WelcomeWindow::loginRequired, [&]() {
        stackedWidget.setCurrentIndex(loginIndex);
    });
    
    QObject::connect(welcomeWindow, &WelcomeWindow::homeRequired, [&]() {
        stackedWidget.setCurrentIndex(homeIndex);
    });
    
    QObject::connect(loginWindow, &LoginWindow::loginSuccess, [&]() {
        stackedWidget.setCurrentIndex(homeIndex);
    });
    
    // 设置初始窗口为欢迎窗口
    stackedWidget.setCurrentIndex(welcomeIndex);
    
    // 最大化显示
    stackedWidget.showMaximized();
    
    return a.exec();
}