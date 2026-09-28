#include <QApplication>
#include <QStackedWidget>
#include <QScreen>
#include "ui/WelcomeWindow.h"
#include "ui/LoginWindow.h"
#include "ui/CompanyWindow.h"
#include "ui/HomeWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("Chat");
    a.setOrganizationName("YourCompany");

    // 创建堆叠窗口
    QStackedWidget stackedWidget;

    // 创建各个页面
    WelcomeWindow* welcomeWindow = new WelcomeWindow;
    LoginWindow* loginWindow = new LoginWindow;
    CompanyWindow* companyWindow = new CompanyWindow;

    stackedWidget.addWidget(welcomeWindow);
    stackedWidget.addWidget(loginWindow);
    stackedWidget.addWidget(companyWindow);

    // HomeWindow 在选完公司之后再创建，
    // 这样它内部的租户/模型请求才能带上已经写进缓存的 companyId
    HomeWindow* homeWindow = nullptr;

    // 把窗口调整成刚好容纳页面内容并居中显示
    auto showFittedPage = [&stackedWidget](QWidget* page) {
        stackedWidget.setCurrentWidget(page);
        stackedWidget.showNormal();
        stackedWidget.resize(page->sizeHint());

        if (QScreen* screen = QApplication::primaryScreen()) {
            const QRect available = screen->availableGeometry();
            stackedWidget.move(available.center() - stackedWidget.rect().center());
        }
    };

    // 全屏显示（页面内部自己把内容卡片居中）
    auto showFullPage = [&stackedWidget](QWidget* page) {
        stackedWidget.setCurrentWidget(page);
        stackedWidget.showMaximized();
    };

    // 全屏显示 HomeWindow
    auto showHomePage = [&stackedWidget, &homeWindow]() {
        if (!homeWindow) {
            homeWindow = new HomeWindow;
            stackedWidget.addWidget(homeWindow);
        }
        stackedWidget.setCurrentWidget(homeWindow);
        stackedWidget.showMaximized();
    };

    // 连接信号
    QObject::connect(welcomeWindow, &WelcomeWindow::loginRequired, [&]() {
        showFittedPage(loginWindow);
    });

    // 已有有效 token 时也要先选公司（会用缓存里的公司 id 自动选中）
    // 进入页面时用当前 token 重新拉一次公司列表
    QObject::connect(welcomeWindow, &WelcomeWindow::companyRequired, [&]() {
        companyWindow->loadCompanyList();
        showFullPage(companyWindow);
    });

    // 登录成功后 token 变了，必须重新拉公司列表
    QObject::connect(loginWindow, &LoginWindow::loginSuccess, [&]() {
        companyWindow->loadCompanyList();
        showFullPage(companyWindow);
    });

    QObject::connect(companyWindow, &CompanyWindow::companySelected, [&](const QString&) {
        showHomePage();
    });

    // 初始窗口为欢迎窗口，全屏展示
    stackedWidget.setCurrentWidget(welcomeWindow);
    stackedWidget.showMaximized();

    return a.exec();
}
