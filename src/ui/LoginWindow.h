#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QMovie>

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccess();
    void backToWelcome();  // 保留但不再使用

private slots:
    void onPasswordLoginTabClicked();
    void onEmailLoginTabClicked();
    void onLoginClicked();
    void onSendCodeClicked();
    void onUsernamePasswordChanged();
    void onEmailChanged(const QString& email);
    void onCodeChanged(const QString& code);

private:
    void setupUI();
    void setupPasswordLoginPanel();
    void setupEmailLoginPanel();
    void updateTabIndicator(int index);
    bool validateEmail(const QString& email);
    void performPasswordLogin();
    void performEmailLogin();
    void setLoading(bool loading);
    
    // 主布局
    QVBoxLayout* mainLayout;
    QWidget* loginContainer;  // 登录框容器
    
    // 页签
    QHBoxLayout* tabLayout;
    QPushButton* passwordLoginTab;
    QPushButton* emailLoginTab;
    QWidget* tabIndicator;  // 页签指示器横线
    
    // 堆叠窗口
    QStackedWidget* stackedWidget;
    
    // 密码登录面板
    QWidget* passwordLoginPanel;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    
    // 邮箱验证码登录面板
    QWidget* emailLoginPanel;
    QLineEdit* emailEdit;
    QHBoxLayout* codeLayout;
    QLineEdit* codeEdit;
    QPushButton* sendCodeBtn;
    
    // 公共按钮
    QPushButton* loginButton;
    QPushButton* registerButton;
    QPushButton* forgotPasswordButton;
    
    // Loading动画
    QMovie* loadingMovie;
    QLabel* loadingLabel;
    
    // 状态
    bool isLoading;
};

#endif // LOGINWINDOW_H