// LeftPanel.h
#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QSettings>
#include <QNetworkAccessManager>  // 添加这个头文件
#include "models/User.h"
#include "models/ApiResponse.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"

// 租户信息结构体
struct TenantInfo {
    QString id;
    QString name;
    QString code;
    QString description;
    int status;
    QDateTime createDate;
    QDateTime updateDate;
    QString createdBy;
    QString updatedBy;
    
    static TenantInfo fromJson(const QJsonObject& json) {
        TenantInfo tenant;
        if (json.contains("id")) tenant.id = json["id"].toString();
        if (json.contains("name")) tenant.name = json["name"].toString();
        if (json.contains("code")) tenant.code = json["code"].toString();
        if (json.contains("description") && !json["description"].isNull()) 
            tenant.description = json["description"].toString();
        if (json.contains("status")) tenant.status = json["status"].toInt();
        if (json.contains("create_date") && !json["create_date"].isNull())
            tenant.createDate = QDateTime::fromString(json["create_date"].toString(), Qt::ISODate);
        if (json.contains("update_date") && !json["update_date"].isNull())
            tenant.updateDate = QDateTime::fromString(json["update_date"].toString(), Qt::ISODate);
        if (json.contains("created_by")) tenant.createdBy = json["created_by"].toString();
        if (json.contains("updated_by") && !json["updated_by"].isNull())
            tenant.updatedBy = json["updated_by"].toString();
        return tenant;
    }
    
    bool isValid() const { return !id.isEmpty(); }
};

// 聊天历史结构体
struct ChatHistory {
    int id;
    QString modelName;
    QString userId;
    QString files;
    QString chatId;
    QString prompt;
    QString systemPrompt;
    QString content;
    QDateTime createTime;
    QString thinkContent;
    QString responseContent;
    
    static ChatHistory fromJson(const QJsonObject& json) {
        ChatHistory chat;
        if (json.contains("id")) chat.id = json["id"].toInt();
        if (json.contains("modelName")) chat.modelName = json["modelName"].toString();
        if (json.contains("userId")) chat.userId = json["userId"].toString();
        if (json.contains("files") && !json["files"].isNull()) chat.files = json["files"].toString();
        if (json.contains("chatId")) chat.chatId = json["chatId"].toString();
        if (json.contains("prompt")) chat.prompt = json["prompt"].toString();
        if (json.contains("SystemPrompt")) chat.systemPrompt = json["SystemPrompt"].toString();
        if (json.contains("content")) chat.content = json["content"].toString();
        if (json.contains("createTime")) 
            chat.createTime = QDateTime::fromString(json["createTime"].toString(), Qt::ISODate);
        if (json.contains("thinkContent") && !json["thinkContent"].isNull())
            chat.thinkContent = json["thinkContent"].toString();
        if (json.contains("responseContent") && !json["responseContent"].isNull())
            chat.responseContent = json["responseContent"].toString();
        return chat;
    }
    
    bool isValid() const { return id > 0; }
};

// 聊天历史分组
struct ChatGroup {
    QString timeLabel;  // 时间标签（刚刚、xx分钟前等）
    QList<ChatHistory> chats;
};

// 用于信号槽传递的自定义类型注册
Q_DECLARE_METATYPE(TenantInfo)
Q_DECLARE_METATYPE(ChatHistory)

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LeftPanel(QWidget *parent = nullptr);
    ~LeftPanel();
    
    // 刷新数据
    void refreshData();
    
    // 重写事件过滤器
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void chatSelected(const ChatHistory& chat);
    void newChatClicked();  // 添加新对话点击信号
    void tenantChanged(const QString& tenantId);  // 租户切换信号
    
private slots:
    void onTenantMenuClicked();
    void onTenantSelected(const TenantInfo& tenant);
    void onScrollToBottom();
    void onChatItemClicked();

    

private:
    void setupUI();
    void setupUserInfoArea();
    void setupNewChatArea();
    void setupChatHistoryArea();
    QIcon createTransparentIcon(const QString &path, double opacity);
    void loadUserInfo();
    void loadTenantList();
    void loadChatHistory(int pageNum = 1, bool append = false);
    QString formatTimeLabel(const QDateTime& time) const;
    QWidget* createChatItemWidget(const ChatHistory& chat);
    void updateChatList(const QList<ChatHistory>& newChats, int total, bool append = false);
    void clearChatList();
    void showTenantPopupMenu();
    TenantInfo createDefaultTenant() const;
    
    // 头像相关的新方法
    void loadAvatar();  // 新增：加载头像
    void createDefaultAvatar();  // 新增：创建默认头像

    // UI组件
    QVBoxLayout* mainLayout;
    QWidget* userInfoWidget;
    QHBoxLayout* userInfoLayout;
    QWidget* newChatWidget;
    QLabel* avatarLabel;
    QVBoxLayout* userTextLayout;
    QLabel* userNameLabel;
    QPushButton* tenantNameBtn;
    QPushButton* tenantArrowBtn;  // 新增：租户选择下拉箭头按钮
    QScrollArea* chatScrollArea;
    QWidget* chatContainer;
    QVBoxLayout* chatLayout;
    
    // 数据
    User currentUser;
    QList<TenantInfo> tenantList;
    TenantInfo currentTenant;
    QList<ChatGroup> chatGroups;
    int currentPageNum;
    int pageSize;
    int totalChats;
    bool isLoadingChats;
    bool hasMoreChats;
    
    // 动画
    QPropertyAnimation* tenantMenuAnimation;
    QParallelAnimationGroup* animationGroup;
};

#endif // LEFTPANEL_H