#include "WelcomeWindow.h"
#include "utils/TokenManager.h"
#include "network/NetworkManager.h"
#include "config/Constants.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QScreen>

WelcomeWindow::WelcomeWindow(QWidget *parent) : QWidget(parent) {
    // 设置窗口大小为全屏
    if (QScreen* screen = QApplication::primaryScreen()) {
        setGeometry(screen->geometry());
    }
    
    // 设置窗口样式
    setStyleSheet(QString("background-color: %1;").arg(Colors::PAGE_BACKGROUND_COLOR.name()));
    
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(Dimens::MIDDLE_MARGIN);
    
    // Logo - 使用资源文件中的图片
    logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/resources/images/logo.png");
    if (!logoPixmap.isNull()) {
        // 根据屏幕大小调整logo尺寸
        int screenWidth = QApplication::primaryScreen()->size().width();
        int logoSize = screenWidth / 4;  // 屏幕宽度的1/4
        logoLabel->setPixmap(logoPixmap.scaled(logoSize, logoSize, 
                                               Qt::KeepAspectRatio, 
                                               Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // 欢迎文字
    welcomeLabel = new QLabel("欢迎使用", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;")
                                .arg(Colors::PRIMARY_COLOR.name())
                                .arg(Dimens::FONT_SIZE_BIG * 2));  // 字体放大
    
    layout->addWidget(logoLabel);
    layout->addWidget(welcomeLabel);
    
    // 创建定时器，设置3秒超时
    timer = new QTimer(this);
    timer->setSingleShot(true);  // 单次触发
    connect(timer, &QTimer::timeout, this, &WelcomeWindow::onTimeout);
    
    // 启动定时器
    timer->start(3000);  // 3秒后触发
}

void WelcomeWindow::onTimeout() {
    // 定时器超时，开始检查token并跳转
    checkTokenAndNavigate();
}

void WelcomeWindow::checkTokenAndNavigate() {
    TokenManager& tokenManager = TokenManager::instance();
    
    if (!tokenManager.hasValidToken()) {
        emit loginRequired();
        return;
    }
    
    // 设置网络请求的token
    NetworkManager::instance().setAuthToken(tokenManager.getToken());
    
    // 获取用户数据
    fetchUserData();
}

void WelcomeWindow::fetchUserData() {
    NetworkManager::instance().get(
        Constants::Endpoints::GET_USER_DATA,
        [this](const ApiResponse& response) {
            if (response.isSuccess() && !response.data.isNull()) {
                // 保存用户数据
                QJsonObject userObj = response.data.toJsonObject();
                User user = User::fromJson(userObj);
                TokenManager::instance().saveUser(user);
                
                // 跳转到主窗口
                emit homeRequired();
            } else {
                // 获取用户数据失败，需要重新登录
                TokenManager::instance().clearToken();
                TokenManager::instance().clearUser();
                emit loginRequired();
            }
        },
        [this](const QString& error) {
            // 网络请求失败，需要重新登录
            TokenManager::instance().clearToken();
            TokenManager::instance().clearUser();
            emit loginRequired();
        }
    );
}