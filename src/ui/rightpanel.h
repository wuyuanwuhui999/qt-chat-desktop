#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QButtonGroup>
#include "theme/Colors.h"
#include "theme/Dimens.h"

class RightPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RightPanel(QWidget *parent = nullptr);

private slots:
    void onDeepThinkToggled();
    void onLanguageToggle();
    void onSearchDocToggled();
    void onDocSelectionToggled();

private:
    void setupUI();
    void updateButtonsStyle();
    
    // 布局
    QVBoxLayout* mainLayout;
    QHBoxLayout* logoLayout;
    
    // Logo和欢迎语
    QLabel* logoLabel;
    QLabel* welcomeLabel;
    
    // 输入框容器（包含输入框和按钮）
    QWidget* inputContainer;
    QVBoxLayout* containerLayout;  // 容器的主布局
    
    QTextEdit* inputEdit;
    
    // 按钮容器
    QWidget* buttonContainer;
    QHBoxLayout* buttonLayout;
    
    // 按钮
    QPushButton* deepThinkBtn;
    QPushButton* languageBtn;
    QPushButton* searchDocBtn;
    QPushButton* docSelectionBtn;
    
    // 状态变量
    bool isDeepThinkSelected;      // 深度思考按钮选中状态
    bool isSearchDocSelected;      // 查询文档按钮选中状态
    bool isDocSelectionVisible;    // 文档选择按钮可见状态
    QString currentLanguage;       // 当前语言：zh/en
};

#endif // RIGHTPANEL_H