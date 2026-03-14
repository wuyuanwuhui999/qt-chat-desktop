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
#include <QTimer>  // 添加QTimer头文件

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccess();

protected:
    // 添加resizeEvent的声明
    void resizeEvent(QResizeEvent* event) override;

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
    void startSendButtonLoading();
    void stopSendButtonLoading();
    
    // 主布局
    QVBoxLayout* mainLayout;
    QWidget* loginContainer;  // 登录框容器
    
    // 页签
    QHBoxLayout* tabLayout;
    QPushButton* passwordLoginTab;
    QPushButton* emailLoginTab;
    
    // 页签指示器（两个独立的指示器）
    QWidget* passwordTabIndicator;  // 密码登录页签指示器
    QWidget* emailTabIndicator;     // 验证码登录页签指示器
    
    int currentTabIndex;    // 当前选中的页签索引
    
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
    QMovie* sendButtonLoadingMovie;
    bool isSendingCode;
    
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