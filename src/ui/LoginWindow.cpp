#include "LoginWindow.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include "theme/Styles.h"
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent) {
    setFixedSize(350, 400);
    setStyleSheet(QString("background-color: %1;").arg(Colors::PAGE_BACKGROUND_COLOR.name()));
    
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(Dimens::MIDDLE_MARGIN);
    mainLayout->setContentsMargins(Dimens::PAGE_PADDING, 
                                   Dimens::PAGE_PADDING, 
                                   Dimens::PAGE_PADDING, 
                                   Dimens::PAGE_PADDING);
    
    // 标题
    titleLabel = new QLabel("登录", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;")
                              .arg(Colors::PRIMARY_COLOR.name())
                              .arg(Dimens::FONT_SIZE_BIG + 4));
    
    // 用户名输入框
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("用户名/邮箱/手机号");
    usernameEdit->setStyleSheet(Styles::inputStyle());
    usernameEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    
    // 密码输入框
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(Styles::inputStyle());
    passwordEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    
    // 登录按钮
    loginButton = new QPushButton("登录", this);
    loginButton->setStyleSheet(Styles::primaryButtonStyle());
    loginButton->setFixedHeight(Dimens::BTN_HEIGHT);
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    
    // 返回按钮
    backButton = new QPushButton("返回", this);
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: " + Colors::SECONDARY_COLOR.name() + ";"
        "   border: none;"
        "   text-decoration: underline;"
        "}"
    );
    connect(backButton, &QPushButton::clicked, this, &LoginWindow::backToWelcome);
    
    mainLayout->addStretch();
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addWidget(loginButton);
    mainLayout->addWidget(backButton);
    mainLayout->addStretch();
}

void LoginWindow::onLoginClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }
    
    // TODO: 实现登录逻辑
    // 这里应该调用登录API，获取token，然后保存并跳转
    
    // 模拟登录成功
    QMessageBox::information(this, "提示", "登录功能待实现");
    
    // 登录成功后：
    // 1. 保存token
    // 2. 保存用户信息
    // 3. 跳转到主窗口
    // emit loginSuccess();
}