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
#include <QMenu>
#include <QAction>
#include <QJsonObject>  // 添加这个头文件
#include <QJsonArray>   // 添加这个头文件
#include "theme/Colors.h"
#include "theme/Dimens.h"

// 模型信息结构体
struct ModelInfo {
    QString id;
    QString modelName;
    QString createTime;
    QString updateTime;
    
    static ModelInfo fromJson(const QJsonObject& json) {
        ModelInfo model;
        if (json.contains("id")) model.id = json["id"].toString();
        if (json.contains("modelName")) model.modelName = json["modelName"].toString();
        if (json.contains("createTime") && !json["createTime"].isNull())
            model.createTime = json["createTime"].toString();
        if (json.contains("updateTime") && !json["updateTime"].isNull())
            model.updateTime = json["updateTime"].toString();
        return model;
    }
    
    bool isValid() const { return !id.isEmpty() && !modelName.isEmpty(); }
};

// 用于信号槽传递的自定义类型注册
Q_DECLARE_METATYPE(ModelInfo)

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
    void onModelMenuClicked();
    void onModelSelected();
    void onSendClicked();          // 发送按钮点击处理

private:
    void setupUI();
    void updateButtonsStyle();
    void loadModelList();
    void showModelPopupMenu();
    void updateCurrentModel(const ModelInfo& model);
    
    
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
    
    // 模型选择相关
    QWidget* modelContainer;        // 模型选择容器
    QHBoxLayout* modelLayout;       // 模型选择布局
    QPushButton* modelNameBtn;      // 模型名称按钮（可点击）
    QPushButton* modelArrowBtn;     // 模型下拉箭头按钮
    
    QList<ModelInfo> modelList;     // 模型列表
    ModelInfo currentModel;          // 当前选中的模型
    
    // 状态变量
    bool isDeepThinkSelected;      // 深度思考按钮选中状态
    bool isSearchDocSelected;      // 查询文档按钮选中状态
    bool isDocSelectionVisible;    // 文档选择按钮可见状态
    QString currentLanguage;       // 当前语言：zh/en

    QPushButton* sendButton;      // 发送按钮
    
};

#endif // RIGHTPANEL_H