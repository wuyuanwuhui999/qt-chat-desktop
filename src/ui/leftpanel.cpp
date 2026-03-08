// LeftPanel.cpp
#include "LeftPanel.h"
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QDateTime>
#include <QDebug>
#include <QFontMetrics>
#include <QJsonArray>

// 引入需要的头文件
#include "utils/TokenManager.h"
#include "network/NetworkManager.h"
#include "config/Constants.h"
#include "models/ApiResponse.h"
#include "models/User.h"

// 时间分类结构体
struct TimeCategory {
    QString displayName;
    QDateTime startTime;
    QDateTime endTime;
    QList<QJsonObject> items;
};

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
    , currentPageNum(1)
    , pageSize(20)
    , totalCount(0)
    , isLoading(false)
    , hasMoreData(true)
{
    setStyleSheet("background-color: #f5f5f5;");
    
    setupUI();
    loadTenantInfo();
    loadChatHistory();
}

void LeftPanel::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 租户信息显示区域
    tenantWidget = new QWidget(this);
    tenantWidget->setFixedHeight(80);
    tenantWidget->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border-bottom: 1px solid #e0e0e0;"
        "}"
    );
    
    QHBoxLayout* tenantLayout = new QHBoxLayout(tenantWidget);
    tenantLayout->setContentsMargins(15, 10, 15, 10);
    
    // 租户头像（使用默认图标）
    tenantAvatarLabel = new QLabel(tenantWidget);
    tenantAvatarLabel->setFixedSize(50, 50);
    tenantAvatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #FFAE00;"
        "   border-radius: 25px;"
        "   color: white;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   qproperty-alignment: AlignCenter;"
        "}"
    );
    tenantAvatarLabel->setText("私");
    
    // 租户信息垂直布局
    QWidget* tenantInfoWidget = new QWidget(tenantWidget);
    QVBoxLayout* tenantInfoLayout = new QVBoxLayout(tenantInfoWidget);
    tenantInfoLayout->setContentsMargins(10, 0, 0, 0);
    tenantInfoLayout->setSpacing(5);
    
    tenantNameLabel = new QLabel(tenantInfoWidget);
    tenantNameLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "}"
    );
    
    tenantCodeLabel = new QLabel(tenantInfoWidget);
    tenantCodeLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 12px;"
        "   color: #999999;"
        "}"
    );
    
    tenantInfoLayout->addWidget(tenantNameLabel);
    tenantInfoLayout->addWidget(tenantCodeLabel);
    tenantInfoLayout->addStretch();
    
    tenantLayout->addWidget(tenantAvatarLabel);
    tenantLayout->addWidget(tenantInfoWidget);
    
    mainLayout->addWidget(tenantWidget);
    
    // 历史记录标题
    QWidget* titleWidget = new QWidget(this);
    titleWidget->setFixedHeight(40);
    titleWidget->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border-bottom: 1px solid #e0e0e0;"
        "}"
    );
    
    QHBoxLayout* titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(15, 0, 15, 0);
    
    QLabel* historyTitleLabel = new QLabel("历史对话", titleWidget);
    historyTitleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "}"
    );
    
    titleLayout->addWidget(historyTitleLabel);
    titleLayout->addStretch();
    
    mainLayout->addWidget(titleWidget);
    
    // 创建滚动区域用于显示历史记录
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   background-color: white;"
        "   border: none;"
        "}"
        "QScrollBar:vertical {"
        "   width: 8px;"
        "   background: #f0f0f0;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #c0c0c0;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #a0a0a0;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );
    
    historyContainer = new QWidget();
    historyContainer->setStyleSheet("background-color: white;");
    historyLayout = new QVBoxLayout(historyContainer);
    historyLayout->setContentsMargins(0, 0, 0, 0);
    historyLayout->setSpacing(0);
    historyLayout->addStretch(); // 添加弹性空间，使内容从顶部开始
    
    scrollArea->setWidget(historyContainer);
    mainLayout->addWidget(scrollArea);
    
    // 连接滚动条的valueChanged信号
    QScrollBar* scrollBar = scrollArea->verticalScrollBar();
    connect(scrollBar, &QScrollBar::valueChanged, this, &LeftPanel::onScrollValueChanged);
    
    // 加载更多提示
    loadingLabel = new QLabel("加载中...", this);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet(
        "QLabel {"
        "   color: #999999;"
        "   font-size: 12px;"
        "   padding: 10px;"
        "}"
    );
    loadingLabel->setVisible(false);
    mainLayout->addWidget(loadingLabel);
}

void LeftPanel::loadTenantInfo()
{
    User currentUser = TokenManager::instance().getUser();
    QString userId = currentUser.id;
    
    // 先尝试从缓存获取租户信息
    QSettings settings("YourCompany", "ChatApp");
    QVariant tenantData = settings.value("tenant_info");
    
    if (tenantData.isValid()) {
        QJsonObject tenantObj = QJsonDocument::fromJson(tenantData.toByteArray()).object();
        currentTenant.id = tenantObj["id"].toString();
        currentTenant.name = tenantObj["name"].toString();
        currentTenant.code = tenantObj["code"].toString();
        currentTenant.status = tenantObj["status"].toInt();
        currentTenant.created_by = tenantObj["created_by"].toString();
    } else {
        // 使用默认租户
        currentTenant.id = userId;  // 租户ID与用户ID相同
        currentTenant.name = "私人空间";
        currentTenant.code = "personal";
        currentTenant.status = 1;
        currentTenant.created_by = "system";
        
        // 保存到缓存
        QJsonObject tenantObj;
        tenantObj["id"] = currentTenant.id;
        tenantObj["name"] = currentTenant.name;
        tenantObj["code"] = currentTenant.code;
        tenantObj["status"] = currentTenant.status;
        tenantObj["created_by"] = currentTenant.created_by;
        
        settings.setValue("tenant_info", QJsonDocument(tenantObj).toJson());
    }
    
    // 更新UI显示
    tenantNameLabel->setText(currentTenant.name);
    tenantCodeLabel->setText(currentTenant.code);
    
    // 更新头像文字（取租户名称第一个字）
    if (!currentTenant.name.isEmpty()) {
        tenantAvatarLabel->setText(currentTenant.name.left(1));
    }
}

void LeftPanel::loadChatHistory()
{
    if (isLoading || !hasMoreData) return;
    
    isLoading = true;
    loadingLabel->setVisible(true);
    
    QString tenantId = currentTenant.id;
    QString url = QString("/service/chat/getChatHistory?tenantId=%1&pageSize=%2&pageNum=%3")
                     .arg(tenantId)
                     .arg(pageSize)
                     .arg(currentPageNum);
    
    NetworkManager::instance().get(
        url,
        [this](const ApiResponse& response) {
            isLoading = false;
            loadingLabel->setVisible(false);
            
            if (response.isSuccess() && !response.data.isNull()) {
                totalCount = response.total;
                
                // 检查是否还有更多数据
                hasMoreData = (pageSize * currentPageNum) < totalCount;
                
                // 解析数据
                QJsonArray dataArray = response.data.toJsonArray();
                if (!dataArray.isEmpty()) {
                    processChatHistory(dataArray);
                    currentPageNum++;
                }
            } else {
                qDebug() << "Failed to load chat history:" << response.message;
            }
        },
        [this](const QString& error) {
            isLoading = false;
            loadingLabel->setVisible(false);
            qDebug() << "Network error when loading chat history:" << error;
        }
    );
}

void LeftPanel::processChatHistory(const QJsonArray& dataArray)
{
    // 按时间分类
    QMap<QString, TimeCategory> categories;
    
    for (const QJsonValue& value : dataArray) {
        QJsonObject item = value.toObject();
        QString createTimeStr = item["createTime"].toString();
        
        if (createTimeStr.isEmpty()) continue;
        
        QDateTime createTime = QDateTime::fromString(createTimeStr, Qt::ISODate);
        if (!createTime.isValid()) continue;
        
        QString categoryKey = getTimeCategoryKey(createTime);
        QString categoryName = getTimeCategoryName(createTime);
        
        if (!categories.contains(categoryKey)) {
            TimeCategory category;
            category.displayName = categoryName;
            categories[categoryKey] = category;
        }
        
        categories[categoryKey].items.append(item);
    }
    
    // 按时间排序分类（最新的在前）
    QStringList sortedKeys = categories.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end(), [](const QString& a, const QString& b) {
        // 根据分类名称排序，假设分类名称包含时间信息
        return a > b;
    });
    
    // 移除之前的拉伸项
    if (historyLayout->count() > 0) {
        QLayoutItem* lastItem = historyLayout->itemAt(historyLayout->count() - 1);
        if (lastItem && lastItem->spacerItem()) {
            delete historyLayout->takeAt(historyLayout->count() - 1);
        }
    }
    
    // 添加新的分类和列表
    for (const QString& key : sortedKeys) {
        const TimeCategory& category = categories[key];
        
        // 添加分类标题（带横线）
        QWidget* categoryWidget = createCategoryWidget(category.displayName);
        historyLayout->addWidget(categoryWidget);
        
        // 添加该分类下的所有聊天项
        for (const QJsonObject& item : category.items) {
            QWidget* chatItemWidget = createChatItemWidget(item);
            historyLayout->addWidget(chatItemWidget);
        }
    }
    
    // 重新添加拉伸项
    historyLayout->addStretch();
}

QString LeftPanel::getTimeCategoryKey(const QDateTime& time)
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = time.secsTo(now);
    
    if (secondsDiff < 60) {  // 1分钟内
        return "just_now";
    } else if (secondsDiff < 3600) {  // 1小时内
        return "minutes_ago";
    } else if (secondsDiff < 30 * 24 * 3600) {  // 30天内
        return "days_ago";
    } else if (secondsDiff < 365 * 24 * 3600) {  // 1年内
        return "months_ago";
    } else {  // 1年以上
        return "years_ago";
    }
}

QString LeftPanel::getTimeCategoryName(const QDateTime& time)
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = time.secsTo(now);
    qint64 minutesDiff = secondsDiff / 60;
    qint64 hoursDiff = minutesDiff / 60;
    qint64 daysDiff = hoursDiff / 24;
    qint64 monthsDiff = daysDiff / 30;
    qint64 yearsDiff = monthsDiff / 12;
    
    if (secondsDiff < 60) {
        return "刚刚";
    } else if (secondsDiff < 3600) {
        return QString("%1分钟前").arg(minutesDiff);
    } else if (secondsDiff < 30 * 24 * 3600) {
        if (daysDiff == 0) daysDiff = 1;
        return QString("%1天前").arg(daysDiff);
    } else if (secondsDiff < 365 * 24 * 3600) {
        if (monthsDiff == 0) monthsDiff = 1;
        return QString("%1个月前").arg(monthsDiff);
    } else {
        if (yearsDiff == 0) yearsDiff = 1;
        return QString("%1年前").arg(yearsDiff);
    }
}

QWidget* LeftPanel::createCategoryWidget(const QString& categoryName)
{
    QWidget* widget = new QWidget(historyContainer);
    widget->setFixedHeight(40);
    
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(15, 0, 15, 0);
    
    // 左侧横线
    QFrame* leftLine = new QFrame(widget);
    leftLine->setFrameShape(QFrame::HLine);
    leftLine->setFixedWidth(30);
    leftLine->setStyleSheet("QFrame { color: #e0e0e0; }");
    
    // 分类名称
    QLabel* nameLabel = new QLabel(categoryName, widget);
    nameLabel->setStyleSheet(
        "QLabel {"
        "   color: #999999;"
        "   font-size: 12px;"
        "}"
    );
    
    // 右侧横线（弹性）
    QFrame* rightLine = new QFrame(widget);
    rightLine->setFrameShape(QFrame::HLine);
    rightLine->setStyleSheet("QFrame { color: #e0e0e0; }");
    
    layout->addWidget(leftLine);
    layout->addWidget(nameLabel);
    layout->addWidget(rightLine, 1);  // 1表示拉伸因子
    
    return widget;
}

QWidget* LeftPanel::createChatItemWidget(const QJsonObject& item)
{
    QWidget* widget = new QWidget(historyContainer);
    widget->setFixedHeight(80);
    widget->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border-bottom: 1px solid #f0f0f0;"
        "}"
        "QWidget:hover {"
        "   background-color: #f9f9f9;"
        "}"
    );
    
    QVBoxLayout* mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(5);
    
    // 提示词（最多两行，超出显示省略号）
    QString prompt = item["prompt"].toString();
    if (prompt.isEmpty()) {
        prompt = "新对话";
    }
    
    QLabel* promptLabel = new QLabel(prompt, widget);
    promptLabel->setWordWrap(true);
    promptLabel->setStyleSheet(
        "QLabel {"
        "   color: #333333;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "}"
    );
    
    // 设置最大行数为2
    QFontMetrics fm(promptLabel->font());
    QString elidedText = fm.elidedText(prompt, Qt::ElideRight, 250, 2);
    promptLabel->setText(elidedText);
    
    // 时间显示
    QString createTimeStr = item["createTime"].toString();
    QDateTime createTime = QDateTime::fromString(createTimeStr, Qt::ISODate);
    QString timeDisplay = getTimeCategoryName(createTime);
    
    QLabel* timeLabel = new QLabel(timeDisplay, widget);
    timeLabel->setStyleSheet(
        "QLabel {"
        "   color: #999999;"
        "   font-size: 11px;"
        "}"
    );
    timeLabel->setAlignment(Qt::AlignRight);
    
    mainLayout->addWidget(promptLabel);
    mainLayout->addWidget(timeLabel, 0, Qt::AlignRight);
    
    return widget;
}

void LeftPanel::onScrollValueChanged(int value)
{
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(sender()->parent());
    if (!scrollArea) return;
    
    QScrollBar* scrollBar = scrollArea->verticalScrollBar();
    if (scrollBar && scrollBar->maximum() - value <= 50) {  // 距离底部50像素内触发加载
        if (hasMoreData && !isLoading) {
            loadChatHistory();
        }
    }
}

void LeftPanel::clearHistoryDisplay()
{
    // 清除所有现有的历史记录显示
    QLayoutItem* item;
    while ((item = historyLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    historyLayout->addStretch();  // 重新添加拉伸项
}

void LeftPanel::refreshHistory()
{
    currentPageNum = 1;
    hasMoreData = true;
    clearHistoryDisplay();
    loadChatHistory();
}