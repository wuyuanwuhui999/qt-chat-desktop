// LeftPanel.cpp
#include "LeftPanel.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include <QJsonArray>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QDebug>
#include <QMenu>
#include <QCursor>
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// 注册自定义类型用于信号槽
static struct MetaTypeRegistration {
    MetaTypeRegistration() {
        qRegisterMetaType<TenantInfo>();
        qRegisterMetaType<ChatHistory>();
    }
} _registration;

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
    , currentPageNum(1)
    , pageSize(20)
    , totalChats(0)
    , isLoadingChats(false)
    , hasMoreChats(true)
{
    setStyleSheet(QString("background-color: 1%;").arg(Colors::WHITE_COLOR.name()));
    setMinimumWidth(250);
    
    setupUI();
    
    // 先设置 token
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        NetworkManager::instance().setAuthToken(token);
        qDebug() << "Set auth token in LeftPanel:" << token.left(20) << "...";
    } else {
        qDebug() << "No token found in LeftPanel";
    }
    
    // 加载数据
    loadUserInfo();
    loadTenantList();
}

LeftPanel::~LeftPanel()
{
}

void LeftPanel::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 用户信息区域 - 移除底部边框
    userInfoWidget = new QWidget(this);
    userInfoWidget->setFixedHeight(Dimens::BAR_HEIGHT);
    userInfoWidget->setStyleSheet(QString(
        "background-color: white;"
        "border-bottom: none;"  // 移除边框
    ));
    
    userInfoLayout = new QHBoxLayout(userInfoWidget);
    userInfoLayout->setContentsMargins(Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING, 0);
    userInfoLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 头像
    avatarLabel = new QLabel(userInfoWidget);
    avatarLabel->setFixedSize(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR);
    avatarLabel->setScaledContents(true);
    
    // 用户文本信息
    userTextLayout = new QVBoxLayout();
    userTextLayout->setContentsMargins(0, 0, 0, 0);
    userTextLayout->setSpacing(Dimens::SMALL_MARGIN);
    
    userNameLabel = new QLabel(userInfoWidget);
    userNameLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold; background-color: transparent;")
                                .arg(Colors::TEXT_COLOR.name())
                                .arg(Dimens::FONT_SIZE_NORMAL));
    
    // 租户名称容器（包含文字和三角形按钮）
    QWidget* tenantContainer = new QWidget(userInfoWidget);
    tenantContainer->setCursor(Qt::PointingHandCursor);
    tenantContainer->setStyleSheet("background-color: transparent; border: none;");
    
    QHBoxLayout* tenantLayout = new QHBoxLayout(tenantContainer);
    tenantLayout->setContentsMargins(0, 0, 0, 0);
    tenantLayout->setSpacing(Dimens::SMALL_MARGIN);
    
    // 租户名称按钮
    tenantNameBtn = new QPushButton(tenantContainer);
    tenantNameBtn->setCursor(Qt::PointingHandCursor);
    tenantNameBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: %1;"
        "   font-size: %2px;"
        "   border: none;"
        "   text-align: left;"
        "   padding: 0;"
        "}"
        "QPushButton:hover {"
        "   color: %3;"
        "}"
    ).arg(Colors::SUB_TITLE_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL - 2)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    // 向下三角形按钮
    tenantArrowBtn = new QPushButton(tenantContainer);
    tenantArrowBtn->setCursor(Qt::PointingHandCursor);
    tenantArrowBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    tenantArrowBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   icon: url(:/resources/images/arrow_down.png);"
        "   icon-size: %1px;"
        "}"
        "QPushButton:hover {"
        "   icon: url(:/resources/images/arrow_down_hover.png);"
        "}"
    ).arg(Dimens::SMALL_ICON_SIZE));
    
    // 如果没有箭头图标，使用文本箭头
    if (tenantArrowBtn->icon().isNull()) {
        tenantArrowBtn->setText("▼");
        tenantArrowBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: transparent;"
            "   color: %1;"
            "   font-size: %2px;"
            "   border: none;"
            "   padding: 0;"
            "}"
            "QPushButton:hover {"
            "   color: %3;"
            "}"
        ).arg(Colors::SUB_TITLE_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL - 2)
         .arg(Colors::PRIMARY_COLOR.name()));
    }
    
    tenantLayout->addWidget(tenantNameBtn);
    tenantLayout->addWidget(tenantArrowBtn);
    tenantLayout->addStretch();
    
    connect(tenantNameBtn, &QPushButton::clicked, this, &LeftPanel::onTenantMenuClicked);
    connect(tenantArrowBtn, &QPushButton::clicked, this, &LeftPanel::onTenantMenuClicked);
    
    userTextLayout->addWidget(userNameLabel);
    userTextLayout->addWidget(tenantContainer);
    
    userInfoLayout->addWidget(avatarLabel);
    userInfoLayout->addLayout(userTextLayout);
    userInfoLayout->addStretch();
    
    // 新对话按钮区域
    QWidget* newChatWidget = new QWidget(this);
    newChatWidget->setFixedHeight(Dimens::BTN_HEIGHT + Dimens::PAGE_PADDING * 2);
    newChatWidget->setStyleSheet("background-color: white;");
    
    QVBoxLayout* newChatLayout = new QVBoxLayout(newChatWidget);
    newChatLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING, 
                                      Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    
     QPushButton* newChatBtn = new QPushButton("+ 开启新对话", newChatWidget);
    newChatBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    newChatBtn->setCursor(Qt::PointingHandCursor);
    newChatBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: white;"
        "   border: none;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: %4;"
        "}"
    ).arg(Colors::PRIMARY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.lighter(110).name()));
    
    connect(newChatBtn, &QPushButton::clicked, [this](){
        qDebug() << "New chat button clicked";
        emit newChatClicked();  // 发射新对话信号
    });
    
    newChatLayout->addWidget(newChatBtn);
    
    // 聊天历史滚动区域
    chatScrollArea = new QScrollArea(this);
    chatScrollArea->setWidgetResizable(true);
    chatScrollArea->setFrameShape(QFrame::NoFrame);
    chatScrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");
    chatScrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "   background-color: transparent;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: " + Colors::GRAY_COLOR.name() + ";"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: " + Colors::PRIMARY_COLOR.name() + ";"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );
    
    chatContainer = new QWidget();
    chatContainer->setStyleSheet("background-color: white;");
    chatContainer->installEventFilter(this);
    
    chatLayout = new QVBoxLayout(chatContainer);
    chatLayout->setContentsMargins(Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    chatLayout->setSpacing(Dimens::PAGE_PADDING);
    chatLayout->addStretch();  // 添加弹簧，使内容从顶部开始
    
    chatScrollArea->setWidget(chatContainer);
    
    // 监听滚动事件
    QScrollBar* scrollBar = chatScrollArea->verticalScrollBar();
    connect(scrollBar, &QScrollBar::valueChanged, [this, scrollBar](int value) {
        if (scrollBar->maximum() > 0 && value >= scrollBar->maximum() - 50) {
            onScrollToBottom();
        }
    });
    
    mainLayout->addWidget(userInfoWidget);
    mainLayout->addWidget(newChatWidget);
    mainLayout->addWidget(chatScrollArea);
}

void LeftPanel::loadUserInfo() {
    currentUser = TokenManager::instance().getUser();
    
    // 设置用户名
    QString displayName = currentUser.username;
    if (displayName.isEmpty()) {
        displayName = currentUser.userAccount;
    }
    if (displayName.isEmpty()) {
        displayName = "用户";
    }
    userNameLabel->setText(displayName);
    
    // 设置头像
    loadAvatar();
}

void LeftPanel::loadAvatar() {
    if (!currentUser.avatar.isEmpty()) {
        // 有头像字段，加载网络头像
        QString avatarUrl = Constants::BASE_URL + currentUser.avatar;
        qDebug() << "Loading avatar from:" << avatarUrl;
        
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QNetworkRequest request(avatarUrl);
        
        // 添加认证头
        QString token = TokenManager::instance().getToken();
        if (!token.isEmpty()) {
            request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
        }
        
        QNetworkReply* reply = manager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, manager]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                pixmap.loadFromData(imageData);
                
                if (!pixmap.isNull()) {
                    // 将图片裁剪为圆形
                    QPixmap rounded = QPixmap(pixmap.size());
                    rounded.fill(Qt::transparent);
                    
                    QPainter painter(&rounded);
                    painter.setRenderHint(QPainter::Antialiasing);
                    
                    QPainterPath path;
                    path.addEllipse(0, 0, pixmap.width(), pixmap.height());
                    painter.setClipPath(path);
                    painter.drawPixmap(0, 0, pixmap);
                    
                    avatarLabel->setPixmap(rounded.scaled(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR, 
                                                          Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    createDefaultAvatar();
                }
            } else {
                qDebug() << "Failed to load avatar:" << reply->errorString();
                createDefaultAvatar();
            }
            
            reply->deleteLater();
            manager->deleteLater();
        });
    } else {
        // 头像为空，使用用户名第一个字母作为头像
        createDefaultAvatar();
    }
}

void LeftPanel::createDefaultAvatar() {
    // 获取用户名第一个字母
    QString initial;
    if (!currentUser.username.isEmpty()) {
        initial = currentUser.username.left(1).toUpper();
    } else if (!currentUser.userAccount.isEmpty()) {
        initial = currentUser.userAccount.left(1).toUpper();
    } else {
        initial = "U";  // 默认用户
    }
    
    QPixmap pixmap(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制圆形背景
    QPainterPath path;
    path.addEllipse(0, 0, Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR);
    painter.fillPath(path, Colors::PRIMARY_COLOR);
    
    // 绘制文字
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(Dimens::MIDDLE_AVATAR * 0.5);  // 稍微调大一点
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, initial);
    
    avatarLabel->setPixmap(pixmap);
}

void LeftPanel::loadTenantList() {
    qDebug() << "Loading tenant list from:" << Constants::Endpoints::GET_USER_TENANT_LIST;
    
    // 确保 token 已设置
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        NetworkManager::instance().setAuthToken(token);
        qDebug() << "Refreshed auth token before tenant list request";
    }
    
    NetworkManager::instance().get(
        Constants::Endpoints::GET_USER_TENANT_LIST,
        [this](const ApiResponse& response) {
            qDebug() << "Tenant list response - status:" << response.status 
                     << "message:" << response.message
                     << "data:" << response.data;
            
            if (response.isSuccess() && !response.data.isNull()) {
                tenantList.clear();
                
                QJsonArray tenantArray = response.data.toJsonArray();
                qDebug() << "Tenant array size:" << tenantArray.size();
                
                for (const QJsonValue& value : tenantArray) {
                    TenantInfo tenant = TenantInfo::fromJson(value.toObject());
                    tenantList.append(tenant);
                    qDebug() << "Tenant:" << tenant.id << tenant.name << tenant.code;
                }
                
                // 确定当前租户 - 使用 TokenManager 的公共方法
                QString cachedTenantId = TokenManager::instance().getValue(Constants::CURRENT_TENANT_ID_KEY).toString();
                qDebug() << "Cached tenant ID:" << cachedTenantId;
                
                bool found = false;
                
                // 检查缓存的租户是否在列表中
                if (!cachedTenantId.isEmpty()) {
                    for (const TenantInfo& tenant : tenantList) {
                        if (tenant.id == cachedTenantId) {
                            currentTenant = tenant;
                            found = true;
                            qDebug() << "Found cached tenant:" << tenant.name;
                            break;
                        }
                    }
                }
                
                // 如果没找到，选择第一条
                if (!found && !tenantList.isEmpty()) {
                    currentTenant = tenantList.first();
                    TokenManager::instance().setValue(Constants::CURRENT_TENANT_ID_KEY, currentTenant.id);
                    qDebug() << "Using first tenant:" << currentTenant.name;
                }
                
                // 如果列表为空，使用默认租户
                if (tenantList.isEmpty()) {
                    currentTenant = createDefaultTenant();
                    qDebug() << "Tenant list empty, using default tenant:" << currentTenant.name;
                }
                
                // 更新租户名称按钮
                tenantNameBtn->setText(currentTenant.name);
                
                // 加载聊天历史
                loadChatHistory(1, false);
            } else {
                qDebug() << "Failed to load tenant list, using default tenant";
                // 使用默认租户
                currentTenant = createDefaultTenant();
                tenantNameBtn->setText(currentTenant.name);
                loadChatHistory(1, false);
            }
        },
        [this](const QString& error) {
            qDebug() << "Network error when loading tenant list:" << error;
            
            // 如果是认证错误，尝试重新获取 token
            if (error.contains("authentication") || error.contains("401")) {
                qDebug() << "Authentication error, token might be expired";
                // 这里可以触发重新登录
            }
            
            // 使用默认租户
            currentTenant = createDefaultTenant();
            tenantNameBtn->setText(currentTenant.name);
            loadChatHistory(1, false);
        }
    );
}

TenantInfo LeftPanel::createDefaultTenant() const {
    TenantInfo tenant;
    tenant.id = currentUser.id;  // 租户ID等于用户ID
    tenant.name = Constants::DefaultTenant::NAME;
    tenant.code = Constants::DefaultTenant::CODE;
    tenant.status = Constants::DefaultTenant::STATUS;
    tenant.createdBy = Constants::DefaultTenant::CREATED_BY;
    return tenant;
}

void LeftPanel::onTenantMenuClicked() {
    qDebug() << "Tenant menu clicked, tenant list size:" << tenantList.size();
    
    if (tenantList.isEmpty()) {
        qDebug() << "Tenant list is empty, reloading...";
        loadTenantList();  // 尝试重新加载租户列表
        return;
    }
    
    showTenantPopupMenu();
}

void LeftPanel::showTenantPopupMenu() {
    if (tenantList.isEmpty()) {
        qDebug() << "Tenant list is empty, cannot show menu";
        return;
    }
    
    qDebug() << "Showing tenant popup menu with" << tenantList.size() << "items";
    
    QMenu menu(this);
    menu.setStyleSheet(QString(
        "QMenu {"
        "   background-color: white;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: %3px;"
        "}"
        "QMenu::item {"
        "   padding: %3px %4px;"
        "   border-radius: %2px;"
        "   color: %5;"
        "   font-size: %6px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: %7;"
        "   color: white;"
        "}"
    ).arg(Colors::LINE_COLOR.name())
     .arg(Dimens::SMALL_MARGIN)
     .arg(Dimens::SMALL_MARGIN)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    for (const TenantInfo& tenant : tenantList) {
        QAction* action = menu.addAction(tenant.name);
        action->setData(tenant.id);
        
        // 标记当前选中的租户
        if (tenant.id == currentTenant.id) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }
        
        connect(action, &QAction::triggered, [this, tenant]() {
            onTenantSelected(tenant);
        });
    }
    
    // 显示菜单
    QPoint pos = tenantNameBtn->mapToGlobal(QPoint(0, tenantNameBtn->height()));
    menu.exec(pos);
}

void LeftPanel::onTenantSelected(const TenantInfo& tenant) {
    qDebug() << "Tenant selected:" << tenant.name << tenant.id;
    
    if (tenant.id == currentTenant.id) return;
    
    currentTenant = tenant;
    tenantNameBtn->setText(tenant.name);
    
    // 保存到缓存 - 使用 TokenManager 的公共方法
    TokenManager::instance().setValue(Constants::CURRENT_TENANT_ID_KEY, tenant.id);
    
    // 清空并重新加载聊天历史
    clearChatList();
    currentPageNum = 1;
    hasMoreChats = true;
    loadChatHistory(1, false);
}

void LeftPanel::loadChatHistory(int pageNum, bool append) {
    if (isLoadingChats || (!hasMoreChats && !append)) return;
    
    isLoadingChats = true;
    
    // 使用 QUrlQuery 正确构建 URL 参数
    QUrl url(Constants::BASE_URL + Constants::Endpoints::GET_CHAT_HISTORY);
    QUrlQuery query;
    query.addQueryItem("tenantId", currentTenant.id);
    query.addQueryItem("pageSize", QString::number(pageSize));
    query.addQueryItem("pageNum", QString::number(pageNum));
    url.setQuery(query);
    
    QString urlString = url.toString();
    // 移除 BASE_URL 部分，因为 NetworkManager 会添加
    urlString = urlString.remove(Constants::BASE_URL);
    
    qDebug() << "Loading chat history from:" << urlString;
    
    // 确保 token 已设置
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        NetworkManager::instance().setAuthToken(token);
    }
    
    NetworkManager::instance().get(
        urlString,
        [this, pageNum, append](const ApiResponse& response) {
            isLoadingChats = false;
            
            qDebug() << "Chat history response - status:" << response.status 
                     << "message:" << response.message
                     << "total:" << response.total
                     << "data:" << response.data;
            
            if (response.isSuccess()) {
                QList<ChatHistory> chats;
                
                if (!response.data.isNull()) {
                    QJsonArray chatArray = response.data.toJsonArray();
                    
                    qDebug() << "Chat array size:" << chatArray.size();
                    
                    for (const QJsonValue& value : chatArray) {
                        ChatHistory chat = ChatHistory::fromJson(value.toObject());
                        if (chat.isValid()) {
                            chats.append(chat);
                            qDebug() << "Chat:" << chat.id << chat.prompt << chat.createTime;
                        }
                    }
                }
                
                totalChats = response.total;
                hasMoreChats = (pageSize * pageNum) < totalChats;
                
                qDebug() << "Loaded" << chats.size() << "chats, total:" << totalChats 
                         << "hasMore:" << hasMoreChats;
                
                updateChatList(chats, totalChats, append);
            } else {
                qDebug() << "Failed to load chat history:" << response.message;
                updateChatList(QList<ChatHistory>(), 0, append);
            }
        },
        [this](const QString& error) {
            isLoadingChats = false;
            qDebug() << "Network error when loading chat history:" << error;
            
            // 如果是400错误，可能是参数问题，尝试不带参数测试
            if (error.contains("Bad Request")) {
                qDebug() << "Bad Request error, checking API endpoint format...";
            }
            
            updateChatList(QList<ChatHistory>(), 0, false);
        }
    );
}

QString LeftPanel::formatTimeLabel(const QDateTime& time) const {
    QDateTime now = QDateTime::currentDateTime();
    qint64 seconds = time.secsTo(now);
    
    if (seconds < 60) {
        return "刚刚";
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        return QString("%1分钟前").arg(minutes);
    } else if (seconds < 86400) {  // 24小时
        int hours = seconds / 3600;
        return QString("%1小时前").arg(hours);
    } else if (seconds < 2592000) {  // 30天
        int days = seconds / 86400;
        return QString("%1天前").arg(days);
    } else if (seconds < 31536000) {  // 365天
        int months = seconds / 2592000;
        return QString("%1个月前").arg(months);
    } else {
        int years = seconds / 31536000;
        return QString("%1年前").arg(years);
    }
}

void LeftPanel::updateChatList(const QList<ChatHistory>& newChats, int total, bool append) {
    if (!append) {
        clearChatList();
    }
    
    if (newChats.isEmpty()) {
        // 显示空状态提示
        QLabel* emptyLabel = new QLabel("暂无历史对话", chatContainer);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(QString(
            "color: %1;"
            "font-size: %2px;"
            "padding: %3px;"
            "background-color: transparent;"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING * 2));
        chatLayout->insertWidget(chatLayout->count() - 1, emptyLabel);
        return;
    }
    
    // 按时间分组
    QMap<QString, QList<ChatHistory>> groupedChats;
    for (const ChatHistory& chat : newChats) {
        QString timeLabel = formatTimeLabel(chat.createTime);
        groupedChats[timeLabel].append(chat);
    }
    
    // 更新显示
    for (auto it = groupedChats.begin(); it != groupedChats.end(); ++it) {
        QString timeLabel = it.key();
        QList<ChatHistory> chats = it.value();
        
        // 添加时间标签
        QLabel* timeLabelWidget = new QLabel(timeLabel, chatContainer);
        timeLabelWidget->setStyleSheet(QString(
            "color: %1;"
            "font-size: %2px;"
            "font-weight: bold;"
            "padding: %3px 0 %3px 0;"
            "background-color: transparent;"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL - 2)
         .arg(Dimens::SMALL_MARGIN));
        chatLayout->insertWidget(chatLayout->count() - 1, timeLabelWidget);
        
        // 添加聊天项
        for (const ChatHistory& chat : chats) {
            QWidget* chatItem = createChatItemWidget(chat);
            chatItem->installEventFilter(this);
            chatLayout->insertWidget(chatLayout->count() - 1, chatItem);
        }
    }
}

QWidget* LeftPanel::createChatItemWidget(const ChatHistory& chat) {
    QWidget* widget = new QWidget(chatContainer);
    widget->setCursor(Qt::PointingHandCursor);
    widget->setFixedHeight(60);  // 固定高度
    widget->setObjectName(QString("chat_item_%1").arg(chat.id));
    widget->setStyleSheet(QString(
        "QWidget {"
        "   background-color: transparent;"
        "   border-radius: %1px;"
        "}"
        "QWidget:hover {"
        "   background-color: %2;"
        "}"
    ).arg(Dimens::SMALL_MARGIN)
     .arg(Colors::SEARCH_INPUT_COLOR.name()));
    
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::SMALL_MARGIN,
                               Dimens::PAGE_PADDING, Dimens::SMALL_MARGIN);
    layout->setSpacing(Dimens::SMALL_MARGIN);
    
    // 提示词
    QLabel* promptLabel = new QLabel(chat.prompt, widget);
    promptLabel->setWordWrap(true);
    promptLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
        "border: none;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    
    // 限制最多两行
    promptLabel->setFixedHeight(40);  // 两行高度
    
    layout->addWidget(promptLabel);
    
    // 存储聊天数据
    widget->setProperty("chat_id", chat.id);
    widget->setProperty("chat_prompt", chat.prompt);
    widget->setProperty("chat_time", chat.createTime.toString(Qt::ISODate));
    widget->setProperty("chat_model", chat.modelName);
    widget->setProperty("chat_content", chat.content);
    
    return widget;
}

void LeftPanel::clearChatList() {
    // 清除所有子控件，保留弹簧
    while (chatLayout->count() > 1) {
        QLayoutItem* item = chatLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->removeEventFilter(this);
            item->widget()->deleteLater();
        }
        delete item;
    }
    chatGroups.clear();
}

void LeftPanel::onScrollToBottom() {
    if (hasMoreChats && !isLoadingChats) {
        currentPageNum++;
        loadChatHistory(currentPageNum, true);
    }
}

void LeftPanel::onChatItemClicked() {
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (!widget) return;
    
    int chatId = widget->property("chat_id").toInt();
    QString prompt = widget->property("chat_prompt").toString();
    QString timeStr = widget->property("chat_time").toString();
    QString modelName = widget->property("chat_model").toString();
    QString content = widget->property("chat_content").toString();
    
    // 创建 ChatHistory 对象
    ChatHistory chat;
    chat.id = chatId;
    chat.prompt = prompt;
    chat.createTime = QDateTime::fromString(timeStr, Qt::ISODate);
    chat.modelName = modelName;
    chat.content = content;
    
    // 发射信号
    emit chatSelected(chat);
}

void LeftPanel::refreshData() {
    // 刷新数据前确保 token 是最新的
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        NetworkManager::instance().setAuthToken(token);
    }
    
    loadUserInfo();
    loadTenantList();
}

bool LeftPanel::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        // 检查是否是聊天项（通过对象名判断）
        if (watched->objectName().startsWith("chat_item_")) {
            QWidget* widget = qobject_cast<QWidget*>(watched);
            if (widget) {
                // 创建并发射信号
                int chatId = widget->property("chat_id").toInt();
                QString prompt = widget->property("chat_prompt").toString();
                QString timeStr = widget->property("chat_time").toString();
                QString modelName = widget->property("chat_model").toString();
                QString content = widget->property("chat_content").toString();
                
                ChatHistory chat;
                chat.id = chatId;
                chat.prompt = prompt;
                chat.createTime = QDateTime::fromString(timeStr, Qt::ISODate);
                chat.modelName = modelName;
                chat.content = content;
                
                emit chatSelected(chat);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}