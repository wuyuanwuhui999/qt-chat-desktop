#include "LoginWindow.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMovie>
#include <QTimer>

#include "LoginWindow.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMovie>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent), isLoading(false) {
    // 不再设置固定大小，让窗口可以最大化
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
    loginContainer->setFixedSize(400, 550);  // 稍微增加高度以容纳新按钮
    
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
    tabIndicator->setStyleSheet(QString("background-color: %1;").arg(Colors::PRIMARY_COLOR.name()));
    
    // 堆叠窗口
    stackedWidget = new QStackedWidget(loginContainer);
    stackedWidget->setFixedHeight(200);  // 固定高度
    
    // 创建loading标签（用于显示加载动画）
    loadingLabel = new QLabel(loginContainer);
    loadingMovie = new QMovie(":/resources/images/loading.gif", QByteArray(), this);
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
    
    // 确保登录框在窗口中心
    // 由于窗口最大化，布局会自动处理居中
}

void LoginWindow::setupPasswordLoginPanel() {
    passwordLoginPanel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(passwordLoginPanel);
    layout->setSpacing(Dimens::PAGE_PADDING);
    layout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, 0);
    
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
    layout->setSpacing(Dimens::PAGE_PADDING);
    layout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, 0);
    
    // 邮箱输入框
    emailEdit = new QLineEdit(emailLoginPanel);
    emailEdit->setPlaceholderText("邮箱");
    emailEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    emailEdit->setStyleSheet(
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
    connect(emailEdit, &QLineEdit::textChanged, this, &LoginWindow::onEmailChanged);
    
    // 验证码输入框和发送按钮的水平布局
    codeLayout = new QHBoxLayout();
    
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
    
    sendCodeBtn = new QPushButton("发送", emailLoginPanel);
    sendCodeBtn->setFixedSize(80, Dimens::INPUT_HEIGHT);
    sendCodeBtn->setCursor(Qt::PointingHandCursor);
    sendCodeBtn->setEnabled(false);
    sendCodeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: " + Colors::DISABLE_COLOR.name() + ";"
        "   color: white;"
        "   border: none;"
        "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
        "   font-size: 14px;"
        "}"
    );
    connect(sendCodeBtn, &QPushButton::clicked, this, &LoginWindow::onSendCodeClicked);
    
    codeLayout->addWidget(codeEdit);
    codeLayout->addWidget(sendCodeBtn);
    
    layout->addWidget(emailEdit);
    layout->addLayout(codeLayout);
    
    stackedWidget->addWidget(emailLoginPanel);
}

void LoginWindow::updateTabIndicator(int index) {
    // 更新页签文字颜色
    QString activeStyle = "QPushButton { background: transparent; border: none; font-size: 16px; padding: 10px 20px; color: " + Colors::PRIMARY_COLOR.name() + "; }";
    QString inactiveStyle = "QPushButton { background: transparent; border: none; font-size: 16px; padding: 10px 20px; color: " + Colors::SUB_TITLE_COLOR.name() + "; }";
    
    passwordLoginTab->setStyleSheet(index == 0 ? activeStyle : inactiveStyle);
    emailLoginTab->setStyleSheet(index == 1 ? activeStyle : inactiveStyle);
    
    // 移动指示器
    int tabWidth = loginContainer->width() / 2;
    int targetX = index * tabWidth;
    
    QPropertyAnimation* animation = new QPropertyAnimation(tabIndicator, "geometry");
    animation->setDuration(200);
    animation->setStartValue(tabIndicator->geometry());
    animation->setEndValue(QRect(targetX, tabIndicator->y(), tabWidth, Dimens::STROKE_WIDTH));
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

// ... 其余函数保持不变（onPasswordLoginTabClicked、onEmailLoginTabClicked等）

void LoginWindow::onPasswordLoginTabClicked() {
    stackedWidget->setCurrentIndex(0);
    updateTabIndicator(0);
}

void LoginWindow::onEmailLoginTabClicked() {
    stackedWidget->setCurrentIndex(1);
    updateTabIndicator(1);
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
    sendCodeBtn->setEnabled(isValid);
    
    if (isValid) {
        sendCodeBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::PRIMARY_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
            "   font-size: 14px;"
            "}"
            "QPushButton:hover {"
            "   background-color: " + Colors::PRIMARY_COLOR.lighter(110).name() + ";"
            "}"
        );
    } else {
        sendCodeBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: " + Colors::DISABLE_COLOR.name() + ";"
            "   color: white;"
            "   border: none;"
            "   border-radius: " + QString::number(Dimens::INPUT_HEIGHT / 2) + "px;"
            "   font-size: 14px;"
            "}"
        );
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
    QString email = emailEdit->text().trimmed();
    
    if (!validateEmail(email)) {
        QMessageBox::warning(this, "提示", "请输入正确的邮箱地址");
        return;
    }
    
    // 发送验证码
    QJsonObject data;
    data["email"] = email;
    
    NetworkManager::instance().post(
        "/service/user/sendEmailVertifyCode",
        data,
        [this](const ApiResponse& response) {
            if (response.isSuccess()) {
                QMessageBox::information(this, "提示", "验证码已发送，请查收邮件");
                
                // 开始倒计时
                QTimer* timer = new QTimer(this);
                int* countdown = new int(60);
                
                connect(timer, &QTimer::timeout, [this, timer, countdown]() {
                    if (*countdown > 0) {
                        sendCodeBtn->setText(QString("%1秒后重发").arg(*countdown));
                        sendCodeBtn->setEnabled(false);
                        (*countdown)--;
                    } else {
                        timer->stop();
                        sendCodeBtn->setText("发送");
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
        "/service/user/login",
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
        "/service/user/loginByEmail",
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
