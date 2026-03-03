#include "LoginWindow.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include "config/Constants.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMovie>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QPainter>
#include <QTransform>

LoginWindow::LoginWindow(QWidget *parent) 
    : QWidget(parent), 
      isLoading(false),
      isSendingCode(false),
      currentTabIndex(0) {
    
    setStyleSheet(QString("background-color: %1;").arg(Colors::PAGE_BACKGROUND_COLOR.name()));
    
    setupUI();
    setupPasswordLoginPanel();
    setupEmailLoginPanel();
    
    // 默认显示密码登录页签
    onPasswordLoginTabClicked();
}

void LoginWindow::setupUI() {
    // 主布局 - 使用弹性布局使登录框居中
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    // 登录框容器 - 固定大小，不随窗口变化
    loginContainer = new QWidget(this);
    loginContainer->setFixedSize(400, 500);
    
    // 设置容器样式（白色背景+圆角）
    loginContainer->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border-radius: 10px;"
        "}"
    );
    
    QVBoxLayout* containerLayout = new QVBoxLayout(loginContainer);
    containerLayout->setSpacing(Dimens::PAGE_PADDING);
    containerLayout->setContentsMargins(Dimens::PAGE_PADDING * 2, 
                                       Dimens::PAGE_PADDING * 2, 
                                       Dimens::PAGE_PADDING * 2, 
                                       Dimens::PAGE_PADDING * 2);
    
    // 页签布局
    tabLayout = new QHBoxLayout();
    tabLayout->setSpacing(0);
    
    passwordLoginTab = new QPushButton("密码登录", loginContainer);
    passwordLoginTab->setCursor(Qt::PointingHandCursor);
    passwordLoginTab->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 16px;"
        "   padding: 10px 20px;"
        "}"
    );
    
    emailLoginTab = new QPushButton("验证码登录", loginContainer);
    emailLoginTab->setCursor(Qt::PointingHandCursor);
    emailLoginTab->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   font-size: 16px;"
        "   padding: 10px 20px;"
        "}"
    );
    
    connect(passwordLoginTab, &QPushButton::clicked, this, &LoginWindow::onPasswordLoginTabClicked);
    connect(emailLoginTab, &QPushButton::clicked, this, &LoginWindow::onEmailLoginTabClicked);
    
    tabLayout->addWidget(passwordLoginTab);
    tabLayout->addWidget(emailLoginTab);
    
    // 页签指示器
    tabIndicator = new QWidget(loginContainer);
    tabIndicator->setFixedHeight(Dimens::STROKE_WIDTH);
    tabIndicator->setStyleSheet("background-color: transparent;"); // 初始透明
    
    // 堆叠窗口
    stackedWidget = new QStackedWidget(loginContainer);
    stackedWidget->setFixedHeight(200);
    
    // 创建loading标签（用于显示加载动画）
    loadingLabel = new QLabel(loginContainer);
    loadingMovie = new QMovie(":/resources/images/loading.png", QByteArray(), this);
    loadingLabel->setMovie(loadingMovie);
    loadingLabel->setFixedSize(20, 20);
    loadingLabel->setVisible(false);
    
    // 登录按钮
    loginButton = new QPushButton("登录", loginContainer);
    loginButton->setFixedHeight(Dimens::BTN_HEIGHT);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setEnabled(false);
    loginButton->setStyleSheet(
        "QPushButton {"
        "   background-color: " + Colors::DISABLE_COLOR.name() + ";"
        "   color: white;"
        "   border: none;"
        "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
        "   font-size: 16px;"
        "}"
    );
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    
    // 注册按钮
    registerButton = new QPushButton("注册账号", loginContainer);
    registerButton->setFixedHeight(Dimens::BTN_HEIGHT);
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setStyleSheet(
        "QPushButton {"
        "   background-color: white;"
        "   color: " + Colors::TEXT_COLOR.name() + ";"
        "   border: 1px solid " + Colors::DISABLE_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::BTN_HEIGHT / 2) + "px;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "   color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    // 预留注册点击事件
    connect(registerButton, &QPushButton::clicked, [](){
        // 后续实现注册功能
    });
    
    // 忘记密码按钮
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
    // 预留忘记密码点击事件
    connect(forgotPasswordButton, &QPushButton::clicked, [](){
        // 后续实现忘记密码功能
    });
    
    // 组装布局
    containerLayout->addLayout(tabLayout);
    containerLayout->addWidget(tabIndicator);
    containerLayout->addWidget(stackedWidget);
    
    // 登录按钮和loading的布局
    QHBoxLayout* loginBtnLayout = new QHBoxLayout();
    loginBtnLayout->addWidget(loginButton);
    loginBtnLayout->addWidget(loadingLabel);
    
    containerLayout->addLayout(loginBtnLayout);
    containerLayout->addWidget(registerButton);
    
    QHBoxLayout* forgotLayout = new QHBoxLayout();
    forgotLayout->setAlignment(Qt::AlignCenter);
    forgotLayout->addWidget(forgotPasswordButton);
    containerLayout->addLayout(forgotLayout);
    
    mainLayout->addWidget(loginContainer);
}

void LoginWindow::setupPasswordLoginPanel() {
    passwordLoginPanel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(passwordLoginPanel);
    layout->setSpacing(Dimens::PAGE_PADDING);  // 设置输入框之间的间距
    layout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, 0);  // 设置顶部边距
    
    // 账号输入框
    usernameEdit = new QLineEdit(passwordLoginPanel);
    usernameEdit->setPlaceholderText("账号");
    usernameEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid " + Colors::DISABLE_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   padding: 0 15px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    connect(usernameEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernamePasswordChanged);
    
    // 密码输入框
    passwordEdit = new QLineEdit(passwordLoginPanel);
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid " + Colors::DISABLE_COLOR.name() + ";"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   padding: 0 15px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
    );
    connect(passwordEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernamePasswordChanged);
    
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    
    stackedWidget->addWidget(passwordLoginPanel);
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
        "   border: 1px solid " + Colors::DISABLE_COLOR.name() + ";"
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
        "   icon: url(:/resources/images/icon_send.png);"
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
        "   border: 1px solid " + Colors::DISABLE_COLOR.name() + ";"
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
    sendButtonLoadingMovie = new QMovie(":/resources/images/loading.png", QByteArray(), this);
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
    QString inactiveStyle = "QPushButton { background: transparent; border: none; font-size: 16px; padding: 10px 20px; color: " + Colors::SUB_TITLE_COLOR.name() + "; }";
    
    passwordLoginTab->setStyleSheet(index == 0 ? activeStyle : inactiveStyle);
    emailLoginTab->setStyleSheet(index == 1 ? activeStyle : inactiveStyle);
    
    // 更新指示器颜色
    if (index == 0) {
        tabIndicator->setStyleSheet(QString("background-color: %1;").arg(Colors::PRIMARY_COLOR.name()));
    } else {
        tabIndicator->setStyleSheet("background-color: transparent;");
    }
    
    // 移动指示器
    int tabWidth = loginContainer->width() / 2;
    int targetX = index * tabWidth;
    
    QPropertyAnimation* animation = new QPropertyAnimation(tabIndicator, "geometry");
    animation->setDuration(200);
    animation->setStartValue(tabIndicator->geometry());
    animation->setEndValue(QRect(targetX, tabIndicator->y(), tabWidth, Dimens::STROKE_WIDTH));
    animation->start(QPropertyAnimation::DeleteWhenStopped);
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
        "   icon: url(:/resources/images/icon_send.png);"
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
    bool hasContent = !usernameEdit->text().isEmpty() && !passwordEdit->text().isEmpty();
    loginButton->setEnabled(hasContent);
    
    if (hasContent) {
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
        loginButton->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::DISABLE_COLOR.name() + ";"
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
            "   icon: url(:/resources/images/icon_send.png);"
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
            "   background-color: " + Colors::DISABLE_COLOR.name() + ";"
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
        loginButton->setText("");
        loginButton->setEnabled(false);
        loadingLabel->setVisible(true);
        loadingMovie->start();
    } else {
        loginButton->setText("登录");
        loginButton->setEnabled(true);
        loadingLabel->setVisible(false);
        loadingMovie->stop();
    }
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
    data["password"] = password;
    
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