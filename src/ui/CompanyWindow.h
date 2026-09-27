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

private slots:
    void onConfirmClicked();

private:
    void setupUI();
    void loadCompanyList();
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
