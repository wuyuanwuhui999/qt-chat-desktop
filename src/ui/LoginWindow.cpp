#include "LoginWindow.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include <QJsonArray>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QDebug>
#include <QMenu>
#include <QCursor>
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

LoginWindow::LoginWindow(QWidget *parent) 
    : QWidget(parent), 
      isLoading(false),
      isSendingCode(false),
      currentTabIndex(0) {
    
    // 设置渐变背景色 - 从浅橙色渐变到白色
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    
    // 创建线性渐变（从上到下）
    QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
    gradient.setColorAt(0.0, Colors::PRIMARY_COLOR.lighter(150));  // 顶部：浅橙色
    gradient.setColorAt(0.5, Colors::PRIMARY_COLOR.lighter(180)); // 中间：更浅的橙色
    gradient.setColorAt(1.0, Colors::WHITE_COLOR);                // 底部：白色
    
    palette.setBrush(QPalette::Window, QBrush(gradient));
    setPalette(palette);
    
    setupUI();
    // 移除重复的 setupPasswordLoginPanel 和 setupEmailLoginPanel 调用
    // 因为这些已经在 setupUI() -> createLoginContainer() -> createStackedWidget() 中调用了
    
    // 初始化显示密码登录页签
    onPasswordLoginTabClicked();
    
    // 修复：初始化时检查一次输入框状态
    QTimer::singleShot(0, this, [this]() {
        onUsernamePasswordChanged();
    });
}

// 重写resizeEvent以在窗口大小改变时更新渐变
void LoginWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    // 更新渐变以适应新的窗口大小
    QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
    gradient.setColorAt(0.0, Colors::PRIMARY_COLOR.lighter(150));
    gradient.setColorAt(0.5, Colors::PRIMARY_COLOR.lighter(180));
    gradient.setColorAt(1.0, Colors::WHITE_COLOR);
    
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QBrush(gradient));
    setPalette(palette);
    
    // 重新居中登录框（如果需要）
    if (loginContainer) {
        int x = (width() - loginContainer->width()) / 2;
        int y = (height() - loginContainer->height()) / 2;
        // 注意：由于使用了布局，通常不需要手动设置位置
    }
}

void LoginWindow::setupUI() {
    // 主布局 - 使用弹性布局使登录框居中
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    // 创建居中布局容器
    QVBoxLayout* centerLayout = createCenterLayout();
    
    // 将居中布局添加到主布局
    mainLayout->addStretch(1);
    mainLayout->addLayout(centerLayout, 0);
    mainLayout->addStretch(1);
}

QVBoxLayout* LoginWindow::createCenterLayout() {
    QVBoxLayout* centerLayout = new QVBoxLayout();
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setSpacing(Dimens::PAGE_PADDING * 2);
    
    // 添加Logo
    centerLayout->addWidget(createLogoLabel(), 0, Qt::AlignCenter);
    
    // 创建并添加登录框容器
    loginContainer = createLoginContainer();
    centerLayout->addWidget(loginContainer);
    
    return centerLayout;
}

QLabel* LoginWindow::createLogoLabel() {
    QLabel* logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/images/logo.png");
    
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaled(Dimens::BIG_AVATAR, Dimens::BIG_AVATAR,
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("📋");
        logoLabel->setStyleSheet(QString("font-size: %1px; color: %2;")
                                .arg(Dimens::BIG_AVATAR)
                                .arg(Colors::PRIMARY_COLOR.name()));
    }
    
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedSize(Dimens::BIG_AVATAR, Dimens::BIG_AVATAR);
    
    return logoLabel;
}

QWidget* LoginWindow::createLoginContainer() {
    QWidget* container = new QWidget(this);
    container->setFixedWidth(400);
    container->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border-radius: 10px;"
        "}"
        "QWidget:hover {"
        "   background-color: white;"
        "}"
    );
    
    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 添加页签
    containerLayout->addLayout(createTabLayout());
    
    // 添加堆叠窗口
    stackedWidget = createStackedWidget();
    containerLayout->addWidget(stackedWidget);
    
    // 添加登录按钮
    loginButton = createLoginButton();
    containerLayout->addWidget(loginButton);
    
    // 添加注册按钮
    registerButton = createRegisterButton();
    containerLayout->addWidget(registerButton);
    
    // 添加忘记密码按钮
    containerLayout->addLayout(createForgotPasswordLayout());
    
    // 添加弹簧
    containerLayout->addStretch();
    
    return container;
}

QHBoxLayout* LoginWindow::createTabLayout() {
    QHBoxLayout* tabContainerLayout = new QHBoxLayout();
    tabContainerLayout->setSpacing(0);
    tabContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    // 密码登录页签
    QWidget* passwordTabContainer = createPasswordTabContainer();
    tabContainerLayout->addWidget(passwordTabContainer);
    
    // 验证码登录页签
    QWidget* emailTabContainer = createEmailTabContainer();
    tabContainerLayout->addWidget(emailTabContainer);
    
    return tabContainerLayout;
}

QWidget* LoginWindow::createPasswordTabContainer() {
    QWidget* container = new QWidget(loginContainer);
    container->setObjectName("passwordTabContainer");
    container->setStyleSheet("QWidget#passwordTabContainer { background-color: transparent; }");
    
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    
    passwordLoginTab = new QPushButton("密码登录", container);
    passwordLoginTab->setCursor(Qt::PointingHandCursor);
    passwordLoginTab->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 16px;"
        "}"
    );
    
    passwordTabIndicator = new QWidget(container);
    passwordTabIndicator->setFixedHeight(Dimens::STROKE_WIDTH);
    passwordTabIndicator->setStyleSheet(QString("background-color: %1;").arg(Colors::PRIMARY_COLOR.name()));
    
    layout->addWidget(passwordLoginTab);
    layout->addWidget(passwordTabIndicator);
    
    connect(passwordLoginTab, &QPushButton::clicked, this, &LoginWindow::onPasswordLoginTabClicked);
    
    return container;
}

QWidget* LoginWindow::createEmailTabContainer() {
    QWidget* container = new QWidget(loginContainer);
    container->setObjectName("emailTabContainer");
    container->setStyleSheet("QWidget#emailTabContainer { background-color: transparent; }");
    
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    
    emailLoginTab = new QPushButton("验证码登录", container);
    emailLoginTab->setCursor(Qt::PointingHandCursor);
    emailLoginTab->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 16px;"
        "}"
    );
    
    emailTabIndicator = new QWidget(container);
    emailTabIndicator->setFixedHeight(Dimens::STROKE_WIDTH);
    emailTabIndicator->setStyleSheet("background-color: transparent;");
    
    layout->addWidget(emailLoginTab);
    layout->addWidget(emailTabIndicator);
    
    connect(emailLoginTab, &QPushButton::clicked, this, &LoginWindow::onEmailLoginTabClicked);
    
    return container;
}

QStackedWidget* LoginWindow::createStackedWidget() {
    QStackedWidget* stacked = new QStackedWidget(loginContainer);
    
    // 创建密码登录面板和验证码登录面板
    passwordLoginPanel = createPasswordLoginPanel();  // 使用 createPasswordLoginPanel 而不是 setupPasswordLoginPanel
    emailLoginPanel = createEmailLoginPanel();        // 使用 createEmailLoginPanel 而不是 setupEmailLoginPanel
    
    stacked->addWidget(passwordLoginPanel);
    stacked->addWidget(emailLoginPanel);
    
    return stacked;
}

QWidget* LoginWindow::createPasswordLoginPanel() {
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Dimens::PAGE_PADDING);
    
    // 账号输入框
    usernameEdit = new QLineEdit(panel);
    usernameEdit->setPlaceholderText("账号");
    usernameEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    usernameEdit->setStyleSheet(createInputStyle());
    // 修复：确保信号连接
    connect(usernameEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernamePasswordChanged);
    
    // 密码输入框
    passwordEdit = new QLineEdit(panel);
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    passwordEdit->setStyleSheet(createInputStyle());
    // 修复：确保信号连接
    connect(passwordEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernamePasswordChanged);
    
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    
    return panel;
}

QWidget* LoginWindow::createEmailLoginPanel() {
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, 0);  // 修改：设置左右边距为0
    layout->setSpacing(Dimens::PAGE_PADDING);
    
    // 邮箱输入容器（包含发送按钮）
    QWidget* emailContainer = createEmailInputContainer();
    layout->addWidget(emailContainer);
    
    // 验证码输入框
    codeEdit = new QLineEdit(panel);
    codeEdit->setPlaceholderText("验证码");
    codeEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    codeEdit->setStyleSheet(createInputStyle());
    connect(codeEdit, &QLineEdit::textChanged, this, &LoginWindow::onCodeChanged);
    
    layout->addWidget(codeEdit);
    
    return panel;
}

QWidget* LoginWindow::createEmailInputContainer() {
    QWidget* container = new QWidget();
    container->setFixedHeight(Dimens::INPUT_HEIGHT);
    
    // 关键修改：给容器设置圆角边框背景，模拟输入框的外观
    container->setStyleSheet(
        "QWidget { "
        "   background-color: white; "
        "   border: 1px solid " + Colors::GRAY_COLOR.name() + "; "
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px; "
        "}"
        "QWidget:focus-within { " // 当内部控件获得焦点时，改变边框颜色
        "   border-color: " + Colors::PRIMARY_COLOR.name() + "; "
        "}"
    );

    QHBoxLayout* layout = new QHBoxLayout(container);
    // 关键修改：设置左右内边距
    // 左边距：Dimens::PAGE_PADDING
    // 上/下边距：0 (因为容器高度已固定)
    // 右边距：Dimens::PAGE_PADDING (满足题目要求的离右边距)
    layout->setContentsMargins(Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING, 0);
    layout->setSpacing(0); // 输入框和按钮之间紧贴，或者根据需要设一个小值

    emailEdit = new QLineEdit(container);
    emailEdit->setPlaceholderText("邮箱");
    emailEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    
    // 修改输入框样式：无边框、无圆角、透明背景，因为容器已经提供了边框
    emailEdit->setStyleSheet(
        "QLineEdit { "
        "   border: none; "
        "   background-color: transparent; "
        "   padding: 0; " 
        "   font-size: 14px; "
        "   color: " + Colors::TEXT_COLOR.name() + "; "
        "}"
        "QLineEdit:focus { "
        "   background-color: transparent; " // 保持透明
        "}"
    );
    
    connect(emailEdit, &QLineEdit::textChanged, this, &LoginWindow::onEmailChanged);

    sendCodeBtn = new QPushButton(container);
    sendCodeBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    sendCodeBtn->setCursor(Qt::PointingHandCursor);
    sendCodeBtn->setEnabled(false);
    
    // 按钮样式保持不变，确保背景透明
    sendCodeBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: transparent; "
        "   border: none; "
        "   icon: url(:/images/icon_send.png); "
        "   icon-size: " + QString::number(Dimens::SMALL_ICON_SIZE) + "px; "
        "}"
        "QPushButton:hover { "
        "   opacity: 0.8; "
        "}"
        "QPushButton:disabled { "
        "   opacity: 0.3; "
        "}"
    );
    
    connect(sendCodeBtn, &QPushButton::clicked, this, &LoginWindow::onSendCodeClicked);

    layout->addWidget(emailEdit, 1); // 设置拉伸因子为1，让输入框占据剩余空间
    layout->addWidget(sendCodeBtn);  // 按钮固定在右侧
    
    // 不需要 setAlignment(sendCodeBtn, Qt::AlignRight)，因为 layout 默认从左到右，
    // 且 emailEdit 设置了拉伸，sendCodeBtn 自然会排在最右边。
    // 右边的间距由 layout->setContentsMargins(..., Dimens::PAGE_PADDING, 0) 控制。

    return container;
}



QString LoginWindow::createInputStyle() {
    return QString(
        "QLineEdit {"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: 0 %3px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: %4;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::INPUT_HEIGHT / 2)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::PRIMARY_COLOR.name());
}

QString LoginWindow::createEmailInputStyle() {
    return QString(
        "QLineEdit {"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: 0 15px;"
        "   padding-right: %3px;"  // 为按钮预留空间
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: %4;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::INPUT_HEIGHT / 2)
     .arg(Dimens::SMALL_ICON_SIZE + Dimens::PAGE_PADDING * 2)  // 按钮宽度 + 左右边距
     .arg(Colors::PRIMARY_COLOR.name());
}

QString LoginWindow::createSendButtonStyle() {
    return QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   icon: url(:/images/icon_send.png);"
        "   icon-size: %1px;"
        "}"
    ).arg(Dimens::SMALL_ICON_SIZE);
}

QPushButton* LoginWindow::createLoginButton() {
    QPushButton* button = new QPushButton("登录", loginContainer);
    button->setFixedHeight(Dimens::BTN_HEIGHT);
    button->setCursor(Qt::PointingHandCursor);
    button->setEnabled(false);
    button->setStyleSheet(createDisabledLoginButtonStyle());
    // 修复：确保这里有连接
    connect(button, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    return button;
}

QString LoginWindow::createDisabledLoginButtonStyle() {
    return QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: white;"
        "   border: none;"
        "   border-radius: %2px;"
        "   font-size: 16px;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2);
}

QPushButton* LoginWindow::createRegisterButton() {
    QPushButton* button = new QPushButton("注册账号", loginContainer);
    button->setFixedHeight(Dimens::BTN_HEIGHT);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(createRegisterButtonStyle());
    connect(button, &QPushButton::clicked, [](){});
    return button;
}

QString LoginWindow::createRegisterButtonStyle() {
    return QString(
        "QPushButton {"
        "   background-color: white;"
        "   color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: %3px;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "   border-color: %4;"
        "   color: %4;"
        "}"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Colors::PRIMARY_COLOR.name());
}

QHBoxLayout* LoginWindow::createForgotPasswordLayout() {
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    
    forgotPasswordButton = new QPushButton("忘记密码？", loginContainer);
    forgotPasswordButton->setCursor(Qt::PointingHandCursor);
    forgotPasswordButton->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: " + Colors::SUB_TITLE_COLOR.name() + ";"
        "   border: none;"
        "   font-size: 14px;"
        "   text-decoration: underline;"
        "}"
        "QPushButton:hover {"
        "   color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    connect(forgotPasswordButton, &QPushButton::clicked, [](){});
    
    layout->addWidget(forgotPasswordButton);
    return layout;
}

// Loading动画相关方法
void LoginWindow::createLoadingLabel() {
    if (!loadingLabel) {
        loadingLabel = new QLabel(loginContainer);
        loadingLabel->setFixedSize(20, 20);
        loadingLabel->setVisible(false);
        loadingLabel->setStyleSheet("background-color: transparent;");
    }
    
    if (!loadingMovie) {
        loadingMovie = new QMovie(":/images/icon_loading.gif", QByteArray(), this);
        loadingLabel->setMovie(loadingMovie);
    }
}

void LoginWindow::createSendButtonLoading() {
    sendButtonLoadingMovie = new QMovie(":/images/loading.gif", QByteArray(), this);
    connect(sendButtonLoadingMovie, &QMovie::frameChanged, [this](int frame) {
        if (isSendingCode) {
            QPixmap pixmap = sendButtonLoadingMovie->currentPixmap();
            QTransform transform;
            transform.rotate(frame * 10);
            pixmap = pixmap.transformed(transform);
            sendCodeBtn->setIcon(QIcon(pixmap));
            sendCodeBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
        }
    });
}

void LoginWindow::setupPasswordLoginPanel() {
    passwordLoginPanel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(passwordLoginPanel);
    layout->setSpacing(Dimens::PAGE_PADDING);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 账号输入框
    usernameEdit = new QLineEdit(passwordLoginPanel);
    usernameEdit->setPlaceholderText("账号");
    usernameEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid " + Colors::GRAY_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   padding: 0 15px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    // 修复：连接textChanged信号
    connect(usernameEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernamePasswordChanged);
    
    // 密码输入框
    passwordEdit = new QLineEdit(passwordLoginPanel);
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid " + Colors::GRAY_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   padding: 0 15px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    // 修复：连接textChanged信号
    connect(passwordEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernamePasswordChanged);
    
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    
    stackedWidget->addWidget(passwordLoginPanel);
    
    // 修复：初始化时检查一次输入框状态
    onUsernamePasswordChanged();
}


void LoginWindow::setupEmailLoginPanel() {
    emailLoginPanel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(emailLoginPanel);
    layout->setSpacing(Dimens::PAGE_PADDING);  // 设置输入框之间的间距
    layout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, 0);  // 设置顶部边距
    
    // 邮箱输入框（包含发送图标）
    QWidget* emailContainer = new QWidget(emailLoginPanel);
    emailContainer->setFixedHeight(Dimens::INPUT_HEIGHT);
    
    QHBoxLayout* emailContainerLayout = new QHBoxLayout(emailContainer);
    emailContainerLayout->setContentsMargins(0, 0, 0, 0);
    emailContainerLayout->setSpacing(0);
    
    emailEdit = new QLineEdit(emailContainer);
    emailEdit->setPlaceholderText("邮箱");
    emailEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    emailEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid " + Colors::GRAY_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   padding: 0 15px;"
        "   padding-right: 40px;"  // 为图标预留空间
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    connect(emailEdit, &QLineEdit::textChanged, this, &LoginWindow::onEmailChanged);
    
    // 创建发送按钮（图标按钮）
    sendCodeBtn = new QPushButton(emailContainer);
    sendCodeBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    sendCodeBtn->setCursor(Qt::PointingHandCursor);
    sendCodeBtn->setEnabled(false);
    sendCodeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   icon: url(:/images/icon_send.png);"
        "   icon-size: " + QString::number(Dimens::SMALL_ICON_SIZE) + "px;"
        "}"
    );
    connect(sendCodeBtn, &QPushButton::clicked, this, &LoginWindow::onSendCodeClicked);
    
    // 将按钮放在输入框容器的最右边
    emailContainerLayout->addWidget(emailEdit);
    emailContainerLayout->addWidget(sendCodeBtn);
    emailContainerLayout->setAlignment(sendCodeBtn, Qt::AlignRight | Qt::AlignVCenter);
    
    // 验证码输入框
    codeEdit = new QLineEdit(emailLoginPanel);
    codeEdit->setPlaceholderText("验证码");
    codeEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    codeEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid " + Colors::GRAY_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   padding: 0 15px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    connect(codeEdit, &QLineEdit::textChanged, this, &LoginWindow::onCodeChanged);
    
    layout->addWidget(emailContainer);
    layout->addWidget(codeEdit);
    
    stackedWidget->addWidget(emailLoginPanel);
    
    // 初始化发送按钮加载动画
    sendButtonLoadingMovie = new QMovie(":/images/loading.gif", QByteArray(), this);
    connect(sendButtonLoadingMovie, &QMovie::frameChanged, [this](int frame) {
        if (isSendingCode) {
            QPixmap pixmap = sendButtonLoadingMovie->currentPixmap();
            // 旋转效果
            QTransform transform;
            transform.rotate(frame * 10);  // 每帧旋转10度
            pixmap = pixmap.transformed(transform);
            sendCodeBtn->setIcon(QIcon(pixmap));
            sendCodeBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
        }
    });
}

void LoginWindow::updateTabIndicator(int index) {
    currentTabIndex = index;
    
    // 更新页签文字颜色
    QString activeStyle = "QPushButton { background: transparent; border: none; font-size: 16px; padding: 10px 20px; color: " + Colors::PRIMARY_COLOR.name() + "; }";
    QString inactiveStyle = "QPushButton { background: transparent; border: none; font-size: 16px; padding: 10px 20px; color: #000000; }"; // 黑色
    
    passwordLoginTab->setStyleSheet(index == 0 ? activeStyle : inactiveStyle);
    emailLoginTab->setStyleSheet(index == 1 ? activeStyle : inactiveStyle);
    
    // 更新指示器：激活的页签显示PRIMARY_COLOR，非激活的页签透明
    if (index == 0) {
        passwordTabIndicator->setStyleSheet(QString("background-color: %1;").arg(Colors::PRIMARY_COLOR.name()));
        emailTabIndicator->setStyleSheet("background-color: transparent;");
    } else {
        passwordTabIndicator->setStyleSheet("background-color: transparent;");
        emailTabIndicator->setStyleSheet(QString("background-color: %1;").arg(Colors::PRIMARY_COLOR.name()));
    }
}

void LoginWindow::startSendButtonLoading() {
    isSendingCode = true;
    sendCodeBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    sendButtonLoadingMovie->start();
}

void LoginWindow::stopSendButtonLoading() {
    isSendingCode = false;
    sendButtonLoadingMovie->stop();
    sendCodeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   icon: url(:/images/icon_send.png);"
        "   icon-size: " + QString::number(Dimens::SMALL_ICON_SIZE) + "px;"
        "}"
    );
}

void LoginWindow::onPasswordLoginTabClicked() {
    stackedWidget->setCurrentIndex(0);
    updateTabIndicator(0);
    
    // 更新登录按钮状态
    onUsernamePasswordChanged();
}

void LoginWindow::onEmailLoginTabClicked() {
    stackedWidget->setCurrentIndex(1);
    updateTabIndicator(1);
    
    // 更新登录按钮状态
    onCodeChanged(codeEdit->text());
}

void LoginWindow::onUsernamePasswordChanged() {
    qDebug() << "onUsernamePasswordChanged called - username:" << usernameEdit->text() 
             << "password:" << QString(passwordEdit->text().length(), '*');
    
    bool hasContent = !usernameEdit->text().isEmpty() && !passwordEdit->text().isEmpty();
    loginButton->setEnabled(hasContent);
    
    if (hasContent) {
        qDebug() << "Enabling login button with PRIMARY_COLOR";
        loginButton->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::PRIMARY_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
            "   font-size: 16px;"
            "}"
            "QPushButton:hover {"
            "   background-color: " + Colors::PRIMARY_COLOR.lighter(110).name() + ";"
            "}"
        );
    } else {
        qDebug() << "Disabling login button with GRAY_COLOR";
        loginButton->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::GRAY_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
            "   font-size: 16px;"
            "}"
        );
    }
}

void LoginWindow::onEmailChanged(const QString& email) {
    bool isValid = validateEmail(email);
    
    if (isValid && !isSendingCode) {
        sendCodeBtn->setEnabled(true);
        sendCodeBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: transparent;"
            "   border: none;"
            "   icon: url(:/images/icon_send.png);"
            "   icon-size: " + QString::number(Dimens::SMALL_ICON_SIZE) + "px;"
            "}"
        );
    } else {
        sendCodeBtn->setEnabled(false);
    }
    
    // 检查验证码是否已输入
    onCodeChanged(codeEdit->text());
}

void LoginWindow::onCodeChanged(const QString& code) {
    bool hasEmail = validateEmail(emailEdit->text());
    bool hasCode = !code.isEmpty();
    loginButton->setEnabled(hasEmail && hasCode);
    
    if (hasEmail && hasCode) {
        loginButton->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::PRIMARY_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
            "   font-size: 16px;"
            "}"
        );
    } else {
        loginButton->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::GRAY_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
            "   font-size: 16px;"
            "}"
        );
    }
}

bool LoginWindow::validateEmail(const QString& email) {
    QRegularExpression regex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    return regex.match(email).hasMatch();
}

void LoginWindow::onSendCodeClicked() {
    if (isSendingCode) return;
    
    QString email = emailEdit->text().trimmed();
    
    if (!validateEmail(email)) {
        QMessageBox::warning(this, "提示", "请输入正确的邮箱地址");
        return;
    }
    
    // 开始加载动画
    startSendButtonLoading();
    
    // 发送验证码
    QJsonObject data;
    data["email"] = email;
    
    NetworkManager::instance().post(
        Constants::Endpoints::SEND_EMAIL_CODE,
        data,
        [this](const ApiResponse& response) {
            stopSendButtonLoading();
            
            if (response.isSuccess()) {
                QMessageBox::information(this, "提示", "验证码已发送，请查收邮件");
                
                // 开始倒计时
                QTimer* timer = new QTimer(this);
                int* countdown = new int(60);
                
                connect(timer, &QTimer::timeout, [this, timer, countdown]() {
                    if (*countdown > 0) {
                        sendCodeBtn->setEnabled(false);
                        (*countdown)--;
                    } else {
                        timer->stop();
                        sendCodeBtn->setEnabled(true);
                        delete countdown;
                        timer->deleteLater();
                    }
                });
                
                timer->start(1000);
            } else {
                QMessageBox::warning(this, "提示", response.message.isEmpty() ? "发送失败" : response.message);
            }
        },
        [this](const QString& error) {
            stopSendButtonLoading();
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}

void LoginWindow::setLoading(bool loading) {
    isLoading = loading;
    
    if (loading) {
        // 加载状态：隐藏文字，显示加载动画
        loginButton->setText("");  // 清空文字
        
        // 确保loadingLabel已创建
        if (!loadingLabel) {
            createLoadingLabel();
        }
        
        // 将loading标签移到按钮内
        loadingLabel->setParent(loginButton);
        loadingLabel->setVisible(true);
        
        if (!loadingMovie) {
            loadingMovie = new QMovie(":/images/icon_loading.gif", QByteArray(), this);
            loadingLabel->setMovie(loadingMovie);
        }
        loadingMovie->start();
        
        // 清除按钮原有的布局（如果有）
        QLayout* oldLayout = loginButton->layout();
        if (oldLayout) {
            // 清空布局中的所有项目
            QLayoutItem* item;
            while ((item = oldLayout->takeAt(0)) != nullptr) {
                if (item->widget() && item->widget() != loadingLabel) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
            delete oldLayout;
        }
        
        // 创建新的水平布局来居中loading标签
        QHBoxLayout* layout = new QHBoxLayout(loginButton);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addStretch();  // 左侧弹簧
        layout->addWidget(loadingLabel);
        layout->addStretch();  // 右侧弹簧
        
        // 设置按钮样式
        loginButton->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::PRIMARY_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
            "   font-size: 16px;"
            "}"
            "QPushButton:hover {"
            "   background-color: " + Colors::PRIMARY_COLOR.lighter(110).name() + ";"
            "}"
        );
        
        loginButton->setEnabled(false);
    } else {
        // 非加载状态：显示文字，隐藏加载动画
        
        if (loadingLabel) {
            loadingLabel->setParent(loginContainer);
            loadingLabel->setVisible(false);
        }
        
        if (loadingMovie) {
            loadingMovie->stop();
        }
        
        // 恢复按钮文字
        loginButton->setText("登录");
        
        // 删除按钮内的布局，恢复默认状态
        QLayout* oldLayout = loginButton->layout();
        if (oldLayout) {
            // 清空布局中的所有项目
            QLayoutItem* item;
            while ((item = oldLayout->takeAt(0)) != nullptr) {
                if (item->widget() && item->widget() != loadingLabel) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
            delete oldLayout;
        }
        
        // 根据输入状态设置按钮样式
        if (stackedWidget->currentIndex() == 0) {
            onUsernamePasswordChanged();
        } else {
            onCodeChanged(codeEdit->text());
        }
        
        loginButton->setEnabled(true);
    }
}

QString md5Encrypt(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
    return QString(hash.toHex());
}

void LoginWindow::onLoginClicked() {
    if (isLoading) return;
    
    if (stackedWidget->currentIndex() == 0) {
        // 密码登录
        performPasswordLogin();
    } else {
        // 验证码登录
        performEmailLogin();
    }
}

void LoginWindow::performPasswordLogin() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }
    
    setLoading(true);
    
    QJsonObject data;
    data["userAccount"] = username;
    data["password"] = md5Encrypt(password);
    
    NetworkManager::instance().post(
        Constants::Endpoints::PASSWORD_LOGIN,
        data,
        [this](const ApiResponse& response) {
            setLoading(false);
            
            if (response.isSuccess()) {
                // 保存token
                if (!response.token.isEmpty()) {
                    TokenManager::instance().saveToken(response.token);
                    NetworkManager::instance().setAuthToken(response.token);
                }
                
                // 保存用户信息
                if (!response.data.isNull()) {
                    QJsonObject userObj = response.data.toJsonObject();
                    User user = User::fromJson(userObj);
                    TokenManager::instance().saveUser(user);
                }
                
                emit loginSuccess();
            } else {
                QMessageBox::warning(this, "登录失败", response.message.isEmpty() ? "账号或密码错误" : response.message);
            }
        },
        [this](const QString& error) {
            setLoading(false);
            QMessageBox::warning(this, "登录失败", "网络错误：" + error);
        }
    );
}

void LoginWindow::performEmailLogin() {
    QString email = emailEdit->text().trimmed();
    QString code = codeEdit->text().trimmed();
    
    if (!validateEmail(email)) {
        QMessageBox::warning(this, "提示", "请输入正确的邮箱地址");
        return;
    }
    
    if (code.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入验证码");
        return;
    }
    
    setLoading(true);
    
    QJsonObject data;
    data["email"] = email;
    data["code"] = code;
    
    NetworkManager::instance().post(
        Constants::Endpoints::EMAIL_LOGIN,
        data,
        [this](const ApiResponse& response) {
            setLoading(false);
            
            if (response.isSuccess()) {
                // 保存token
                if (!response.token.isEmpty()) {
                    TokenManager::instance().saveToken(response.token);
                    NetworkManager::instance().setAuthToken(response.token);
                }
                
                // 保存用户信息
                if (!response.data.isNull()) {
                    QJsonObject userObj = response.data.toJsonObject();
                    User user = User::fromJson(userObj);
                    TokenManager::instance().saveUser(user);
                }
                
                emit loginSuccess();
            } else {
                QMessageBox::warning(this, "登录失败", response.message.isEmpty() ? "验证码错误" : response.message);
            }
        },
        [this](const QString& error) {
            setLoading(false);
            QMessageBox::warning(this, "登录失败", "网络错误：" + error);
        }
    );
}