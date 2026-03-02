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
#include <QDebug>

WelcomeWindow::WelcomeWindow(QWidget *parent) : QWidget(parent) {
    // 设置窗口大小为全屏
    if (QScreen* screen = QApplication::primaryScreen()) {
        setGeometry(screen->geometry());
    }
    
    // 设置窗口样式
    setStyleSheet(QString("background-color: %1;").arg(Colors::PAGE_BACKGROUND_COLOR.name()));
    
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(Dimens::SMALL_MARGIN);  // 使用较小的间距
    
    // 添加顶部拉伸，确保垂直居中
    layout->addStretch();
    
    // Logo - 使用 Dimens.h 中的 BIG_AVATAR 大小
    logoLabel = new QLabel(this);
    
    QPixmap logoPixmap(":/images/logo.png");
    if (!logoPixmap.isNull()) {
        int logoSize = Dimens::BIG_AVATAR;
        logoLabel->setPixmap(logoPixmap.scaled(logoSize, logoSize, 
                                               Qt::KeepAspectRatio, 
                                               Qt::SmoothTransformation));
    } else {
        logoLabel->setText("LOGO");
        logoLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;")
                                .arg(Colors::PRIMARY_COLOR.name())
                                .arg(Dimens::FONT_SIZE_XL));
    }
    
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // 欢迎文字 - 使用 PRIMARY_COLOR
    welcomeLabel = new QLabel("欢迎使用", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;")
                                .arg(Colors::PRIMARY_COLOR.name())
                                .arg(Dimens::FONT_SIZE_XL));
    
    layout->addWidget(logoLabel);
    layout->addWidget(welcomeLabel);
    
    // 添加底部拉伸，确保完全居中
    layout->addStretch();
    
    // 创建定时器
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &WelcomeWindow::onTimeout);
    timer->start(3000);
}

// 实现 onTimeout 函数
void WelcomeWindow::onTimeout() {
    qDebug() << "WelcomeWindow timeout, checking token...";
    checkTokenAndNavigate();
}

void WelcomeWindow::checkTokenAndNavigate() {
    TokenManager& tokenManager = TokenManager::instance();
    
    if (!tokenManager.hasValidToken()) {
        qDebug() << "No valid token, going to login";
        emit loginRequired();
        return;
    }
    
    // 设置网络请求的token
    NetworkManager::instance().setAuthToken(tokenManager.getToken());
    
    // 获取用户数据
    fetchUserData();
}

void WelcomeWindow::fetchUserData() {
    qDebug() << "Fetching user data...";
    
    NetworkManager::instance().get(
        Constants::Endpoints::GET_USER_DATA,
        [this](const ApiResponse& response) {
            if (response.isSuccess() && !response.data.isNull()) {
                qDebug() << "User data fetched successfully";
                // 保存用户数据
                QJsonObject userObj = response.data.toJsonObject();
                User user = User::fromJson(userObj);
                TokenManager::instance().saveUser(user);
                
                // 跳转到主窗口
                emit homeRequired();
            } else {
                qDebug() << "Failed to fetch user data:" << response.message;
                // 获取用户数据失败，需要重新登录
                TokenManager::instance().clearToken();
                TokenManager::instance().clearUser();
                emit loginRequired();
            }
        },
        [this](const QString& error) {
            qDebug() << "Network error:" << error;
            // 网络请求失败，需要重新登录
            TokenManager::instance().clearToken();
            TokenManager::instance().clearUser();
            emit loginRequired();
        }
    );
}
