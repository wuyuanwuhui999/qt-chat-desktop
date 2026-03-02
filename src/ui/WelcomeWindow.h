#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

class WelcomeWindow : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);

signals:
    void loginRequired();
    void homeRequired();

private slots:
    void onTimeout();  // 定时器超时处理

private:
    void checkTokenAndNavigate();
    void fetchUserData();
    
    QVBoxLayout* layout;
    QLabel* logoLabel;
    QLabel* welcomeLabel;
    QTimer* timer;  // 添加定时器
};

#endif // WELCOMEWINDOW_H