// LeftPanel.h
#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QJsonObject>
#include <QScrollArea>

// 租户信息结构体
struct TenantInfo {
    QString id;
    QString name;
    QString code;
    QString description;
    int status;
    QString created_by;
};

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LeftPanel(QWidget *parent = nullptr);
    
    // 刷新历史记录
    void refreshHistory();

private slots:
    void onScrollValueChanged(int value);

private:
    void setupUI();
    void loadTenantInfo();
    void loadChatHistory();
    void processChatHistory(const QJsonArray& dataArray);
    QString getTimeCategoryKey(const QDateTime& time);
    QString getTimeCategoryName(const QDateTime& time);
    QWidget* createCategoryWidget(const QString& categoryName);
    QWidget* createChatItemWidget(const QJsonObject& item);
    void clearHistoryDisplay();
    
    // UI组件
    QVBoxLayout* mainLayout;
    QVBoxLayout* historyLayout;
    QWidget* historyContainer;
    QWidget* tenantWidget;
    QLabel* tenantAvatarLabel;
    QLabel* tenantNameLabel;
    QLabel* tenantCodeLabel;
    QLabel* loadingLabel;
    
    // 租户信息
    TenantInfo currentTenant;
    
    // 分页相关
    int currentPageNum;
    int pageSize;
    int totalCount;
    bool isLoading;
    bool hasMoreData;
};

#endif // LEFTPANEL_H