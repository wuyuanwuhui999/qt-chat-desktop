#ifndef COMPANYWINDOW_H
#define COMPANYWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QButtonGroup>
#include <QList>
#include "models/Company.h"

// 公司选择页：登录成功后进入，选定公司后进入 HomeWindow
class CompanyWindow : public QWidget {
    Q_OBJECT

public:
    explicit CompanyWindow(QWidget *parent = nullptr);

signals:
    void companySelected(const QString& companyId);

public:
    // 加载公司列表。
    // 必须在进入页面时调用（登录成功 / 已有 token 通过校验后），
    // 不能只在构造函数里拉一次：启动时往往还没有有效 token，
    // 那一次请求会 401，之后又没有人重新请求，页面就一直是空的。
    void loadCompanyList();

private slots:
    void onConfirmClicked();

private:
    void setupUI();
    void addCompanyItem(const Company& company);
    void clearCompanyList();
    void selectCompanyById(const QString& companyId);
    void updateConfirmButtonState();

    QVBoxLayout* mainLayout;
    QWidget* card;
    QVBoxLayout* cardLayout;
    QLabel* titleLabel;
    QScrollArea* scrollArea;
    QWidget* listContainer;
    QVBoxLayout* listLayout;
    QPushButton* confirmBtn;

    QButtonGroup* radioGroup;
    QList<Company> companyList;
    Company selectedCompany;
};

#endif // COMPANYWINDOW_H
