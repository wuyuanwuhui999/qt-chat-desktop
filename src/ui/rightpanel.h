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
#include <QJsonObject>
#include <QJsonArray>
#include <QWebSocket>
#include <QAbstractSocket>
#include "theme/Colors.h"
#include "theme/Dimens.h"

struct ChatHistory;

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

// 消息块结构体
struct MessageBlock {
    int id;
    bool isThinking;
    QString thinkContent;
    QString responseContent;
    QLabel* thinkLabel;
    QLabel* responseLabel;
    QWidget* widget;
    
    MessageBlock() : id(0), isThinking(false), thinkLabel(nullptr), responseLabel(nullptr), widget(nullptr) {}
};

Q_DECLARE_METATYPE(MessageBlock)

class RightPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RightPanel(QWidget *parent = nullptr);
    ~RightPanel();
    
    // 清空所有消息
    void clearAllMessages();
    
    // 刷新当前租户的系统提示词
    void refreshSystemPrompt(const QString& tenantId);
    // 在 RightPanel.h 的 public 部分添加
    void loadChatHistory(const ChatHistory& chat);

private slots:
    void onDeepThinkToggled();
    void onLanguageToggle();
    void onSearchDocToggled();
    void onDocSelectionToggled();
    void onModelMenuClicked();
    void onModelSelected();
    void onSendClicked();
    void onInputTextChanged();
    
    // WebSocket相关槽函数
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onTextMessageReceived(const QString& message);
    void onWebSocketError(QAbstractSocket::SocketError error);

    void onEditPromptClicked();  // 编辑提示词按钮点击

    void onUploadDocClicked();


private:
    // UI 设置方法 - 拆分后的方法
    void setupUI();
    void setupMessageArea();
    void setupLogoArea();
    void setupInputArea();
    void setupInputEdit();
    QWidget* createTopButtonArea();
    void setupModelSelection();
    void setupButtonArea();
    void createFunctionButtons(QHBoxLayout* layout);
    void createActionButtons(QHBoxLayout* layout);
    QWidget* createBottomContainer();
    
    // 原有方法
    void updateButtonsStyle();
    void updateSendButtonStyle(bool hasText);
    void loadModelList();
    void showModelPopupMenu();
    void updateCurrentModel(const ModelInfo& model);
    QString generateChatId();
    void generateNewChatId();
    void connectWebSocket();
    void addUserMessage(const QString& content);
    void addAssistantMessage();
    void updateCurrentMessage(const QString& content);
    
    // 系统提示词相关方法
    void loadSystemPrompt(const QString& tenantId);
    QString getSystemPromptCacheKey(const QString& tenantId) const;
    void saveSystemPromptToCache(const QString& tenantId, const QString& prompt);
    QString getSystemPromptFromCache(const QString& tenantId) const;
    void fetchSystemPromptFromServer(const QString& tenantId);
    void setSystemPrompt(const QString& prompt);
    void enterEditMode();
    void exitEditMode();
    
    // 布局
    QVBoxLayout* mainLayout;
    
    // 消息显示区域
    QScrollArea* messageScrollArea;
    QWidget* messageContainer;
    QVBoxLayout* messageLayout;
    
    // Logo和欢迎语容器
    QWidget* logoContainer;
    QLabel* logoLabel;
    QLabel* welcomeLabel;
    
    // 输入框容器
    QWidget* inputContainer;
    QVBoxLayout* containerLayout;
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
    QWidget* modelContainer;
    QHBoxLayout* modelLayout;
    QPushButton* modelNameBtn;
    QPushButton* modelArrowBtn;
    
    QList<ModelInfo> modelList;
    ModelInfo currentModel;
    
    // 状态变量
    bool isDeepThinkSelected;
    bool isSearchDocSelected;
    bool isDocSelectionVisible;
    QString currentLanguage;
    
    QPushButton* sendButton;
    
    // WebSocket相关
    QWebSocket* webSocket;
    bool isReceivingMessage;
    QString currentChatId;
    
    // 消息相关
    int currentMessageId;
    MessageBlock currentMessageBlock;

    QPushButton* editPromptBtn;  // 编辑提示词按钮
    bool isEditingPrompt;        // 是否正在编辑提示词
    QString savedInputContent;   // 保存的输入框内容
    QString currentTenantId;     // 当前租户ID
    QString currentSystemPrompt; // 当前系统提示词

    QPushButton* uploadDocBtn;
};

#endif // RIGHTPANEL_H