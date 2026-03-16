#include "RightPanel.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include "ui/LeftPanel.h"
#include <QDebug>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>
#include <QJsonArray>
#include <QCursor>
#include <QMenu>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QDateTime>
#include <QPropertyAnimation>

// 注册自定义类型用于信号槽
static struct MetaTypeRegistration {
    MetaTypeRegistration() {
        qRegisterMetaType<MessageBlock>();
    }
} _registration;

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
    , isDeepThinkSelected(false)
    , isSearchDocSelected(false)
    , isDocSelectionVisible(false)
    , isEditingPrompt(false)
    , currentLanguage("zh")
    , webSocket(nullptr)
    , isReceivingMessage(false)
    , currentMessageId(0)
{
    setStyleSheet("background-color: white;");
    
    // 生成初始会话ID
    generateNewChatId();
    
    setupUI();
    
    // 加载模型列表
    loadModelList();
    
    // 连接输入框文本变化信号
    connect(inputEdit, &QTextEdit::textChanged, this, &RightPanel::onInputTextChanged);
    
    // 初始化时消息区域隐藏
    messageScrollArea->hide();
    
    // 获取当前租户ID
    currentTenantId = TokenManager::instance().getValue(Constants::CURRENT_TENANT_ID_KEY).toString();
    if (!currentTenantId.isEmpty()) {
        loadSystemPrompt(currentTenantId);
    }
}

void RightPanel::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Dimens::PAGE_PADDING, 
                                   Dimens::PAGE_PADDING, 
                                   Dimens::PAGE_PADDING, 
                                   Dimens::PAGE_PADDING);
    mainLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 创建各个UI组件
    setupMessageArea();
    setupLogoArea();
    setupInputArea();
    
    // 创建底部容器
    QWidget* bottomContainer = createBottomContainer();
    
    // 添加到主布局
    mainLayout->addWidget(messageScrollArea, 1);  // 消息区域占满剩余空间
    mainLayout->addWidget(bottomContainer, 1);    // 底部容器也设置拉伸因子，与消息区域平分空间
}

void RightPanel::setupMessageArea() {
    // ========== 消息显示区域 ==========
    messageScrollArea = new QScrollArea(this);
    messageScrollArea->setWidgetResizable(true);
    messageScrollArea->setFrameShape(QFrame::NoFrame);
    messageScrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");
    messageScrollArea->verticalScrollBar()->setStyleSheet(
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
    
    messageContainer = new QWidget();
    messageContainer->setStyleSheet("background-color: white;");
    
    messageLayout = new QVBoxLayout(messageContainer);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    messageLayout->setSpacing(Dimens::PAGE_PADDING);
    messageLayout->addStretch();  // 添加弹簧，使消息从底部开始
    
    messageScrollArea->setWidget(messageContainer);
}

void RightPanel::setupLogoArea() {
    // ========== Logo和欢迎语区域（初始显示，背景透明）==========
    logoContainer = new QWidget(this);
    logoContainer->setStyleSheet("background-color: transparent;");
    
    QVBoxLayout* logoContainerLayout = new QVBoxLayout(logoContainer);
    logoContainerLayout->setContentsMargins(0, 0, 0, 0);
    logoContainerLayout->setSpacing(Dimens::PAGE_PADDING);
    
    QHBoxLayout* logoLayout = new QHBoxLayout();
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // Logo
    logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/images/logo.png");
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaled(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR,
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("📋");
        logoLabel->setStyleSheet(QString("font-size: %1px; color: %2;")
                                .arg(Dimens::MIDDLE_AVATAR)
                                .arg(Colors::PRIMARY_COLOR.name()));
    }
    logoLabel->setFixedSize(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR);
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // 欢迎语 - 使用黑色字体
    welcomeLabel = new QLabel("今天有什么可以帮到你", this);
    welcomeLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "font-weight: bold;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_XL));
    
    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(welcomeLabel);
    
    logoContainerLayout->addStretch();
    logoContainerLayout->addLayout(logoLayout);
    logoContainerLayout->addStretch();
}

void RightPanel::setupInputArea() {
    // ========== 输入框容器（固定在底部）==========
    inputContainer = new QWidget(this);
    inputContainer->setFixedWidth(900);
    inputContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    inputContainer->setStyleSheet(QString(
        "QWidget {"
        "   background-color: white;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BIG_BORDER_RADIUS));
    
    containerLayout = new QVBoxLayout(inputContainer);
    containerLayout->setContentsMargins(Dimens::PAGE_PADDING, 
                                        Dimens::PAGE_PADDING, 
                                        Dimens::PAGE_PADDING, 
                                        Dimens::PAGE_PADDING);
    containerLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 创建输入框
    setupInputEdit();
    
    // 创建顶部按钮区域
    QWidget* topButtonContainer = createTopButtonArea();
    
    // 创建底部按钮区域
    setupButtonArea();
    
    containerLayout->addWidget(topButtonContainer);
    containerLayout->addWidget(inputEdit);
    containerLayout->addWidget(buttonContainer);
}

void RightPanel::setupInputEdit() {
    // 输入框 - placeholder改为默认文本
    inputEdit = new QTextEdit(inputContainer);
    inputEdit->setPlaceholderText("给chat发送消息");
    inputEdit->setFrameStyle(QFrame::NoFrame);
    inputEdit->setMinimumHeight(Dimens::INPUT_HEIGHT);
    inputEdit->setMaximumHeight(Dimens::INPUT_MAX_HEIGHT);
    inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inputEdit->setStyleSheet(QString(
        "QTextEdit {"
        "   background-color: transparent;"
        "   color: %1;"
        "   font-size: %2px;"
        "   border: none;"
        "   padding: 0px;"
        "}"
        "QTextEdit::placeholder {"
        "   color: %3;"
        "}"
        "QScrollBar:vertical {"
        "   background-color: transparent;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: %4;"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: %5;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::GRAY_COLOR.name())
     .arg(Colors::GRAY_COLOR.name())
     .arg(Colors::PRIMARY_COLOR.name()));
}

QWidget* RightPanel::createTopButtonArea() {
    // ========== 顶部按钮区域 ==========
    QWidget* topButtonContainer = new QWidget(inputContainer);
    topButtonContainer->setStyleSheet("background-color: transparent; border: none;");
    
    QHBoxLayout* topButtonLayout = new QHBoxLayout(topButtonContainer);
    topButtonLayout->setContentsMargins(0, 0, 0, 0);
    topButtonLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 创建模型选择区域
    setupModelSelection();
    
    // 语言切换按钮
    languageBtn = new QPushButton("中文", topButtonContainer);
    languageBtn->setCursor(Qt::PointingHandCursor);
    languageBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    languageBtn->setLayoutDirection(Qt::RightToLeft);
    languageBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: %1;"
        "   border: none;"
        "   font-size: %2px;"
        "   padding: 0 %3px;"
        "   text-align: right;"
        "}"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Dimens::PAGE_PADDING));
    
    QPixmap switchPixmap(":/images/icon_switch.png");
    if (!switchPixmap.isNull()) {
        languageBtn->setIcon(QIcon(switchPixmap));
        languageBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
    } else {
        qDebug() << "Switch icon not found, using text fallback";
        languageBtn->setText("中文 🔄");
    }
    
    topButtonLayout->addWidget(modelContainer);
    topButtonLayout->addStretch();
    topButtonLayout->addWidget(languageBtn);
    
    return topButtonContainer;
}

void RightPanel::setupModelSelection() {
    // 模型选择容器
    modelContainer = new QWidget(inputContainer);
    modelContainer->setCursor(Qt::PointingHandCursor);
    modelContainer->setStyleSheet("background-color: transparent; border: none;");
    
    modelLayout = new QHBoxLayout(modelContainer);
    modelLayout->setContentsMargins(0, 0, 0, 0);
    modelLayout->setSpacing(Dimens::SMALL_MARGIN);
    
    modelNameBtn = new QPushButton("加载模型中...", modelContainer);
    modelNameBtn->setCursor(Qt::PointingHandCursor);
    modelNameBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    modelNameBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: %1;"
        "   border: none;"
        "   font-size: %2px;"
        "   font-weight: bold;"
        "   text-align: left;"
        "   padding: 0;"
        "}"
        "QPushButton:hover {"
        "   color: %3;"
        "}"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    modelArrowBtn = new QPushButton(modelContainer);
    modelArrowBtn->setCursor(Qt::PointingHandCursor);
    modelArrowBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    
    QPixmap arrowPixmap(":/images/icon_down.png");
    if (!arrowPixmap.isNull()) {
        modelArrowBtn->setIcon(QIcon(arrowPixmap));
        modelArrowBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
        modelArrowBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    } else {
        modelArrowBtn->setText("▼");
        modelArrowBtn->setStyleSheet(QString(
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
        ).arg(Colors::TEXT_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Colors::PRIMARY_COLOR.name()));
    }
    
    modelLayout->addWidget(modelNameBtn);
    modelLayout->addWidget(modelArrowBtn);
    
    connect(modelNameBtn, &QPushButton::clicked, this, &RightPanel::onModelMenuClicked);
    connect(modelArrowBtn, &QPushButton::clicked, this, &RightPanel::onModelMenuClicked);
}

void RightPanel::setupButtonArea() {
    // ========== 底部按钮区域 ==========
    buttonContainer = new QWidget(inputContainer);
    buttonContainer->setStyleSheet("background-color: transparent; border: none;");
    
    buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 创建各个功能按钮
    createFunctionButtons();
    
    // 创建编辑和发送按钮
    createActionButtons();
    
    buttonLayout->addWidget(deepThinkBtn);
    buttonLayout->addWidget(searchDocBtn);
    buttonLayout->addWidget(docSelectionBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(editPromptBtn);
    buttonLayout->addSpacing(Dimens::PAGE_PADDING);
    buttonLayout->addWidget(sendButton);
}

void RightPanel::createFunctionButtons() {
    // 深度思考按钮
    deepThinkBtn = new QPushButton("深度思考", buttonContainer);
    deepThinkBtn->setCursor(Qt::PointingHandCursor);
    deepThinkBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    deepThinkBtn->setMinimumWidth(100);
    deepThinkBtn->setCheckable(true);
    deepThinkBtn->setChecked(false);
    deepThinkBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: white;"
        "   color: %1;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "   padding: 0 %4px;"
        "}"
        "QPushButton:checked {"
        "   background-color: %5;"
        "   color: white;"
        "   border: 1px solid %5;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    // 查询文档按钮
    searchDocBtn = new QPushButton("查询文档", buttonContainer);
    searchDocBtn->setCursor(Qt::PointingHandCursor);
    searchDocBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    searchDocBtn->setMinimumWidth(100);
    searchDocBtn->setCheckable(true);
    searchDocBtn->setChecked(false);
    searchDocBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: white;"
        "   color: %1;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "   padding: 0 %4px;"
        "}"
        "QPushButton:checked {"
        "   background-color: %5;"
        "   color: white;"
        "   border: 1px solid %5;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    // 选择文档按钮
    docSelectionBtn = new QPushButton("选择文档", buttonContainer);
    docSelectionBtn->setCursor(Qt::PointingHandCursor);
    docSelectionBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    docSelectionBtn->setMinimumWidth(100);
    docSelectionBtn->setVisible(false);
    docSelectionBtn->setCheckable(true);
    docSelectionBtn->setChecked(false);
    docSelectionBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: white;"
        "   color: %1;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "   padding: 0 %4px;"
        "}"
        "QPushButton:checked {"
        "   background-color: %5;"
        "   color: white;"
        "   border: 1px solid %5;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    connect(deepThinkBtn, &QPushButton::toggled, this, &RightPanel::onDeepThinkToggled);
    connect(searchDocBtn, &QPushButton::toggled, this, &RightPanel::onSearchDocToggled);
    connect(docSelectionBtn, &QPushButton::toggled, this, &RightPanel::onDocSelectionToggled);
}

void RightPanel::createActionButtons() {
    // 编辑提示词按钮
    editPromptBtn = new QPushButton(buttonContainer);
    editPromptBtn->setCursor(Qt::PointingHandCursor);
    editPromptBtn->setFixedSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE);
    
    // 设置初始图标为 icon_edit.png (带50%透明度)
    QPixmap editPixmap(":/images/icon_edit.png");
    if (!editPixmap.isNull()) {
        QPixmap transparentPixmap(editPixmap.size());
        transparentPixmap.fill(Qt::transparent);
        QPainter painter(&transparentPixmap);
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, editPixmap);
        editPromptBtn->setIcon(QIcon(transparentPixmap));
        editPromptBtn->setIconSize(QSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE));
    } else {
        editPromptBtn->setText("✏️");
    }

    editPromptBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   opacity: 0.8;"
        "}"
    ));
    
    // 发送按钮
    sendButton = new QPushButton(buttonContainer);
    sendButton->setCursor(Qt::PointingHandCursor);
    sendButton->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    sendButton->setEnabled(false);
    updateSendButtonStyle(false);
    
    // 加载原图并转换为白色
    QPixmap sendPixmap(":/images/icon_send.png");
    if (!sendPixmap.isNull()) {
        // 创建白色版本的图标
        QPixmap whitePixmap(sendPixmap.size());
        whitePixmap.fill(Qt::transparent);
        
        QPainter painter(&whitePixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 方法1：使用 CompositionMode_SourceIn 将图片转换为白色
        painter.drawPixmap(0, 0, sendPixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(whitePixmap.rect(), Qt::white);
        painter.end();
        
        sendButton->setIcon(QIcon(whitePixmap));
        sendButton->setIconSize(QSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE));
    } else {
        sendButton->setText("➤");
    }
    
    connect(editPromptBtn, &QPushButton::clicked, this, &RightPanel::onEditPromptClicked);
    connect(sendButton, &QPushButton::clicked, this, &RightPanel::onSendClicked);
}

QWidget* RightPanel::createBottomContainer() {
    // ========== 底部容器（包含logoContainer和inputContainer）==========
    QWidget* bottomContainer = new QWidget(this);
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomContainer);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(Dimens::PAGE_PADDING * 2);

    // 添加顶部拉伸，使logoContainer垂直居中
    bottomLayout->addStretch();

    // logo容器
    bottomLayout->addWidget(logoContainer, 0, Qt::AlignCenter);

    // 输入框容器居中
    QHBoxLayout* inputCenterLayout = new QHBoxLayout();
    inputCenterLayout->addStretch();
    inputCenterLayout->addWidget(inputContainer);
    inputCenterLayout->addStretch();
    bottomLayout->addLayout(inputCenterLayout);

    // 添加底部拉伸，与顶部拉伸配合实现垂直居中
    bottomLayout->addStretch();
    
    return bottomContainer;
}

// 以下为原有的其他方法，保持不变
void RightPanel::refreshSystemPrompt(const QString& tenantId) {
    if (tenantId.isEmpty()) return;
    
    currentTenantId = tenantId;
    loadSystemPrompt(tenantId);
}

void RightPanel::loadSystemPrompt(const QString& tenantId) {
    // 先从缓存中获取
    QString cachedPrompt = getSystemPromptFromCache(tenantId);
    
    if (!cachedPrompt.isEmpty()) {
        // 缓存中有，直接使用
        setSystemPrompt(cachedPrompt);
        qDebug() << "Loaded system prompt from cache for tenant:" << tenantId;
    } else {
        // 缓存中没有，从服务器获取
        fetchSystemPromptFromServer(tenantId);
    }
}

QString RightPanel::getSystemPromptCacheKey(const QString& tenantId) const {
    return Constants::SYSTEM_PROMPT_PREFIX + tenantId;
}

void RightPanel::saveSystemPromptToCache(const QString& tenantId, const QString& prompt) {
    TokenManager::instance().setValue(getSystemPromptCacheKey(tenantId), prompt);
}

QString RightPanel::getSystemPromptFromCache(const QString& tenantId) const {
    return TokenManager::instance().getValue(getSystemPromptCacheKey(tenantId)).toString();
}

void RightPanel::fetchSystemPromptFromServer(const QString& tenantId) {
    qDebug() << "Fetching system prompt from server for tenant:" << tenantId;
    
    QString urlString = Constants::Endpoints::GET_DEFAULT_PROMPT_BY_TENANT_ID + 
                        "?tenantId=" + tenantId;
    
    NetworkManager::instance().get(
        urlString,
        [this, tenantId](const ApiResponse& response) {
            if (response.isSuccess() && !response.data.isNull()) {
                // 解析返回的JSON对象
                QJsonObject dataObj = response.data.toJsonObject();
                QString prompt = dataObj["prompt"].toString();
                
                if (!prompt.isEmpty()) {
                    // 保存到缓存并设置
                    saveSystemPromptToCache(tenantId, prompt);
                    setSystemPrompt(prompt);
                    qDebug() << "Fetched system prompt from server for tenant:" << tenantId;
                } else {
                    // 返回的prompt为空，使用默认提示词
                    setSystemPrompt(Constants::DEFAULT_SYSTEM_PROMPT);
                    saveSystemPromptToCache(tenantId, Constants::DEFAULT_SYSTEM_PROMPT);
                    qDebug() << "Server returned empty prompt, using default for tenant:" << tenantId;
                }
            } else {
                // 请求失败或data为null，使用默认提示词
                qDebug() << "Failed to fetch system prompt from server, using default for tenant:" << tenantId;
                setSystemPrompt(Constants::DEFAULT_SYSTEM_PROMPT);
                saveSystemPromptToCache(tenantId, Constants::DEFAULT_SYSTEM_PROMPT);
            }
        },
        [this, tenantId](const QString& error) {
            qDebug() << "Network error when fetching system prompt:" << error;
            // 网络错误，使用默认提示词
            setSystemPrompt(Constants::DEFAULT_SYSTEM_PROMPT);
            saveSystemPromptToCache(tenantId, Constants::DEFAULT_SYSTEM_PROMPT);
        }
    );
}

void RightPanel::setSystemPrompt(const QString& prompt) {
    currentSystemPrompt = prompt;
    // 不设置到placeholder中
}

void RightPanel::enterEditMode() {
    // 保存当前输入框内容
    savedInputContent = inputEdit->toPlainText();
    
    // 在输入框中显示系统提示词
    inputEdit->setPlainText(currentSystemPrompt);
    
    // 不添加边框，保持无边框样式
    // 移除添加边框的代码
    
    // 切换图标为退出编辑模式
    QPixmap exitEditPixmap(":/images/icon_exit_edit.png");
    if (!exitEditPixmap.isNull()) {
        QPixmap transparentPixmap(exitEditPixmap.size());
        transparentPixmap.fill(Qt::transparent);
        QPainter painter(&transparentPixmap);
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, exitEditPixmap);
        editPromptBtn->setIcon(QIcon(transparentPixmap));
        editPromptBtn->setIconSize(QSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE));
    }
    
    isEditingPrompt = true;
    
    // 禁用其他按钮
    deepThinkBtn->setEnabled(false);
    searchDocBtn->setEnabled(false);
    modelContainer->setEnabled(false);
    languageBtn->setEnabled(false);
    sendButton->setEnabled(false);
}

void RightPanel::exitEditMode() {
    // 获取编辑后的系统提示词
    QString newSystemPrompt = inputEdit->toPlainText().trimmed();
    
    if (!newSystemPrompt.isEmpty() && newSystemPrompt != currentSystemPrompt) {
        // 更新系统提示词并保存到缓存
        currentSystemPrompt = newSystemPrompt;
        saveSystemPromptToCache(currentTenantId, currentSystemPrompt);
        qDebug() << "System prompt updated and saved to cache for tenant:" << currentTenantId;
    }
    
    // 恢复输入框内容
    inputEdit->setPlainText(savedInputContent);
    savedInputContent.clear();
    
    // 不需要恢复边框，因为本来就没有边框
    // 移除恢复边框的代码
    
    // 切换回编辑图标
    QPixmap editPixmap(":/images/icon_edit.png");
    if (!editPixmap.isNull()) {
        QPixmap transparentPixmap(editPixmap.size());
        transparentPixmap.fill(Qt::transparent);
        QPainter painter(&transparentPixmap);
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, editPixmap);
        editPromptBtn->setIcon(QIcon(transparentPixmap));
        editPromptBtn->setIconSize(QSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE));
    }
    
    isEditingPrompt = false;
    
    // 启用其他按钮
    deepThinkBtn->setEnabled(true);
    searchDocBtn->setEnabled(true);
    modelContainer->setEnabled(true);
    languageBtn->setEnabled(true);
    sendButton->setEnabled(!inputEdit->toPlainText().trimmed().isEmpty());
}

void RightPanel::updateSendButtonStyle(bool hasText) {
    if (hasText && !isReceivingMessage && !isEditingPrompt) {
        sendButton->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   border: none;"
            "   border-radius: %2px;"
            "}"
            "QPushButton:hover {"
            "   background-color: %3;"
            "}"
        ).arg(Colors::PRIMARY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Colors::PRIMARY_COLOR.lighter(110).name()));
        sendButton->setEnabled(true);
    } else {
        sendButton->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   border: none;"
            "   border-radius: %2px;"
            "}"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2));
        sendButton->setEnabled(false);
    }
}

void RightPanel::onInputTextChanged() {
    bool hasText = !inputEdit->toPlainText().trimmed().isEmpty();
    updateSendButtonStyle(hasText);
}

QString RightPanel::generateChatId() {
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 32;
    
    QString randomString;
    for(int i = 0; i < randomStringLength; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        randomString.append(possibleCharacters.at(index));
    }
    return randomString;
}

void RightPanel::generateNewChatId() {
    currentChatId = generateChatId();
    qDebug() << "Generated new chat ID:" << currentChatId;
}

void RightPanel::clearAllMessages() {
    // 清除所有消息，保留弹簧
    while (messageLayout->count() > 1) {
        QLayoutItem* item = messageLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // 隐藏消息区域，显示logo
    messageScrollArea->hide();
    logoContainer->show();
    
    // 重置输入框
    inputEdit->clear();
    onInputTextChanged();
    
    // 生成新的会话ID
    generateNewChatId();
    
    // 重置接收状态
    isReceivingMessage = false;
    if (webSocket) {
        webSocket->close();
        webSocket->deleteLater();
        webSocket = nullptr;
    }
    
    // 重新加载当前租户的系统提示词
    currentTenantId = TokenManager::instance().getValue(Constants::CURRENT_TENANT_ID_KEY).toString();
    if (!currentTenantId.isEmpty()) {
        loadSystemPrompt(currentTenantId);
    }
}

void RightPanel::connectWebSocket() {
    if (webSocket) {
        webSocket->close();
        webSocket->deleteLater();
    }
    
    webSocket = new QWebSocket();
    
    connect(webSocket, &QWebSocket::connected, this, &RightPanel::onWebSocketConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &RightPanel::onWebSocketDisconnected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &RightPanel::onTextMessageReceived);
    connect(webSocket, &QWebSocket::errorOccurred, this, &RightPanel::onWebSocketError);
    
    // 构建WebSocket URL
    QString token = TokenManager::instance().getToken();
    QString wsUrl = Constants::WEBSOCKET_CHAT_URL.arg(token);
    
    qDebug() << "Connecting to WebSocket:" << wsUrl;
    webSocket->open(QUrl(wsUrl));
}

void RightPanel::onWebSocketConnected() {
    qDebug() << "WebSocket connected successfully";
    
    // 显示消息区域，隐藏logo
    messageScrollArea->show();
    logoContainer->hide();
    
    // 构建发送消息
    QJsonObject message;
    message["modelId"] = currentModel.id;
    message["chatId"] = currentChatId;
    
    // 使用当前的系统提示词
    message["systemPrompt"] = currentSystemPrompt;
    
    message["type"] = isSearchDocSelected ? "document" : "";
    message["docIds"] = QJsonArray();  // 暂时为空数组
    
    QString inputValue = inputEdit->toPlainText().trimmed();
    message["prompt"] = inputValue;
    message["showThink"] = isDeepThinkSelected;
    
    // 获取当前租户ID
    QString tenantId = TokenManager::instance().getValue(Constants::CURRENT_TENANT_ID_KEY).toString();
    message["tenantId"] = tenantId;
    
    message["language"] = currentLanguage;
    
    QJsonDocument doc(message);
    QString messageStr = doc.toJson(QJsonDocument::Compact);
    
    qDebug() << "Sending message:" << messageStr;
    webSocket->sendTextMessage(messageStr);
    
    // 在界面上显示用户消息
    addUserMessage(inputValue);
    
    // 清空输入框
    inputEdit->clear();
    onInputTextChanged();
    
    // 开始接收消息，发送按钮变灰
    isReceivingMessage = true;
    updateSendButtonStyle(false);
}

void RightPanel::addUserMessage(const QString& content) {
    QWidget* messageWidget = new QWidget(messageContainer);
    messageWidget->setObjectName(QString("user_msg_%1").arg(++currentMessageId));
    
    // 创建水平布局
    QHBoxLayout* layout = new QHBoxLayout(messageWidget);
    layout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                               Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    layout->setSpacing(Dimens::PAGE_PADDING);
    
    // 关键修改：确保布局顶部对齐
    layout->setAlignment(Qt::AlignTop); 

    // 用户头像
    QLabel* avatarLabel = new QLabel(messageWidget);
    avatarLabel->setFixedSize(Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
    avatarLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // 重要：设置头像的对齐方式为顶部对齐
    avatarLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    
    User currentUser = TokenManager::instance().getUser();
    
    // 生成默认头像逻辑
    QPixmap pixmap(Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
    painter.fillPath(path, Colors::PRIMARY_COLOR);
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(Dimens::SMALL_AVATAR * 0.5);
    painter.setFont(font);
    QString initial = currentUser.username.left(1).toUpper();
    if (initial.isEmpty()) initial = "U";
    painter.drawText(pixmap.rect(), Qt::AlignCenter, initial);
    avatarLabel->setPixmap(pixmap);

    // 消息内容容器 - 使用QWidget包装消息内容，使其可以独立控制对齐
    QWidget* contentContainer = new QWidget(messageWidget);
    contentContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    QVBoxLayout* contentContainerLayout = new QVBoxLayout(contentContainer);
    contentContainerLayout->setContentsMargins(0, 0, 0, 0);
    contentContainerLayout->setSpacing(0);
    // 设置内容容器布局为顶部对齐
    contentContainerLayout->setAlignment(Qt::AlignTop);

    // 消息内容
    QLabel* contentLabel = new QLabel(content, contentContainer);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: %3;"
        "border-radius: %4px;"
        "padding: %5px;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::SMALL_MARGIN)
     .arg(Dimens::PAGE_PADDING));
    
    contentLabel->setMaximumWidth(QWIDGETSIZE_MAX); 
    contentLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft); // 文本内容顶部对齐

    // 将消息内容添加到内容容器的布局中
    contentContainerLayout->addWidget(contentLabel);
    contentContainerLayout->addStretch(); // 添加弹簧，确保内容靠上

    // 添加头像到主布局
    layout->addWidget(avatarLabel, 0, Qt::AlignTop); // 显式指定头像顶部对齐
    
    // 添加内容容器到主布局，设置拉伸因子为1
    layout->addWidget(contentContainer, 1);

    // 插入到弹簧之前
    messageLayout->insertWidget(messageLayout->count() - 1, messageWidget);

    // 滚动到底部
    QTimer::singleShot(100, this, [this]() {
        messageScrollArea->verticalScrollBar()->setValue(
            messageScrollArea->verticalScrollBar()->maximum()
        );
    });
}

void RightPanel::addAssistantMessage() {
    MessageBlock block;
    block.id = ++currentMessageId;
    block.isThinking = false;
    block.thinkContent.clear();
    block.responseContent.clear();

    QWidget* messageWidget = new QWidget(messageContainer);
    messageWidget->setObjectName(QString("assistant_msg_%1").arg(block.id));

    QHBoxLayout* layout = new QHBoxLayout(messageWidget);
    layout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                               Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    layout->setSpacing(Dimens::PAGE_PADDING);
    
    // 【修改点1】设置布局整体顶部对齐，确保头像和消息内容顶部对齐
    layout->setAlignment(Qt::AlignTop); 

    // 助手头像
    QLabel* avatarLabel = new QLabel(messageWidget);
    avatarLabel->setFixedSize(Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
    // 【修改点2】防止头像被拉伸变形
    avatarLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QPixmap pixmap(Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
    painter.fillPath(path, Colors::GRAY_COLOR);
    painter.setPen(Qt::white);
    QFont font;
    font.setPixelSize(Dimens::SMALL_AVATAR * 0.5);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "AI");
    avatarLabel->setPixmap(pixmap);

    // 消息内容容器
    QWidget* contentWidget = new QWidget(messageWidget);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(Dimens::SMALL_MARGIN);

    // 思考内容标签
    QLabel* thinkLabel = new QLabel(contentWidget);
    thinkLabel->setWordWrap(true);
    thinkLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: %3;"
        "border-radius: %4px;"
        "padding: %5px;"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::SMALL_MARGIN)
     .arg(Dimens::PAGE_PADDING));
    
    // 【修改点3】移除最大宽度限制，允许占满剩余空间
    // thinkLabel->setMaximumWidth(600); 
    thinkLabel->setMaximumWidth(QWIDGETSIZE_MAX); 
    thinkLabel->hide();

    // 响应内容标签
    QLabel* responseLabel = new QLabel(contentWidget);
    responseLabel->setWordWrap(true);
    responseLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: %3;"
        "border-radius: %4px;"
        "padding: %5px;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::SMALL_MARGIN)
     .arg(Dimens::PAGE_PADDING));
    
    // 【修改点4】移除最大宽度限制，允许占满剩余空间
    // responseLabel->setMaximumWidth(600);
    responseLabel->setMaximumWidth(QWIDGETSIZE_MAX);

    contentLayout->addWidget(thinkLabel);
    contentLayout->addWidget(responseLabel);

    // 添加头像到布局
    layout->addWidget(avatarLabel);
    
    // 【修改点5】添加内容容器到布局，并设置拉伸因子为1
    // 这样 contentWidget 会占据除了头像和间距之外的所有剩余空间
    layout->addWidget(contentWidget, 1); 
    
    // 原有的 stretch 可以省略，因为 contentWidget 已经设置了 stretch factor
    
    messageLayout->insertWidget(messageLayout->count() - 1, messageWidget);

    block.thinkLabel = thinkLabel;
    block.responseLabel = responseLabel;
    block.widget = messageWidget;
    currentMessageBlock = block;

    // 滚动到底部
    QTimer::singleShot(100, this, [this]() {
        messageScrollArea->verticalScrollBar()->setValue(
            messageScrollArea->verticalScrollBar()->maximum()
        );
    });
}

void RightPanel::updateCurrentMessage(const QString& content) {
    if (!currentMessageBlock.widget) return;
    
    if (content.startsWith("<think>") && content.endsWith("</think>")) {
        // 思考内容
        currentMessageBlock.isThinking = true;
        QString thinkContent = content.mid(7, content.length() - 15);  // 去掉<think>和</think>
        currentMessageBlock.thinkContent += thinkContent;
        
        if (currentMessageBlock.thinkLabel) {
            currentMessageBlock.thinkLabel->setText(currentMessageBlock.thinkContent);
            currentMessageBlock.thinkLabel->show();
        }
    } else if (content == "[done]" || content == "[completed]") {
        // 消息完成
        isReceivingMessage = false;
        updateSendButtonStyle(!inputEdit->toPlainText().trimmed().isEmpty());
        
        // 重置当前消息块
        currentMessageBlock = MessageBlock();
    } else {
        // 响应内容
        if (currentMessageBlock.isThinking) {
            currentMessageBlock.isThinking = false;
        }
        currentMessageBlock.responseContent += content;
        
        if (currentMessageBlock.responseLabel) {
            currentMessageBlock.responseLabel->setText(currentMessageBlock.responseContent);
        }
    }
    
    // 滚动到底部
    QTimer::singleShot(50, this, [this]() {
        messageScrollArea->verticalScrollBar()->setValue(
            messageScrollArea->verticalScrollBar()->maximum()
        );
    });
}

void RightPanel::onWebSocketDisconnected() {
    qDebug() << "WebSocket disconnected";
    
    if (isReceivingMessage) {
        isReceivingMessage = false;
        updateSendButtonStyle(!inputEdit->toPlainText().trimmed().isEmpty());
    }
}

void RightPanel::onTextMessageReceived(const QString& message) {
    qDebug() << "Received message:" << message.left(100) << "...";
    
    // 如果没有当前消息块，创建一个新的
    if (!currentMessageBlock.widget) {
        addAssistantMessage();
    }
    
    // 更新消息
    updateCurrentMessage(message);
}

void RightPanel::onWebSocketError(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error << webSocket->errorString();
    
    isReceivingMessage = false;
    updateSendButtonStyle(!inputEdit->toPlainText().trimmed().isEmpty());
}

void RightPanel::loadModelList() {
    qDebug() << "Loading model list from:" << Constants::Endpoints::GET_MODEL_LIST;
    
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        NetworkManager::instance().setAuthToken(token);
    }
    
    NetworkManager::instance().get(
        Constants::Endpoints::GET_MODEL_LIST,
        [this](const ApiResponse& response) {
            qDebug() << "Model list response - status:" << response.status 
                     << "message:" << response.message;
            
            if (response.isSuccess() && !response.data.isNull()) {
                modelList.clear();
                
                QJsonArray modelArray = response.data.toJsonArray();
                qDebug() << "Model array size:" << modelArray.size();
                
                for (const QJsonValue& value : modelArray) {
                    ModelInfo model = ModelInfo::fromJson(value.toObject());
                    if (model.isValid()) {
                        modelList.append(model);
                        qDebug() << "Model:" << model.id << model.modelName;
                    }
                }
                
                if (modelList.isEmpty()) {
                    qDebug() << "Model list is empty";
                    modelNameBtn->setText("无可用模型");
                    return;
                }
                
                QString cachedModelId = TokenManager::instance().getValue(Constants::SELECTED_MODEL_ID_KEY).toString();
                qDebug() << "Cached model ID:" << cachedModelId;
                
                bool found = false;
                
                if (!cachedModelId.isEmpty()) {
                    for (const ModelInfo& model : modelList) {
                        if (model.id == cachedModelId) {
                            updateCurrentModel(model);
                            found = true;
                            qDebug() << "Found cached model:" << model.modelName;
                            break;
                        }
                    }
                }
                
                if (!found && !modelList.isEmpty()) {
                    updateCurrentModel(modelList.first());
                    qDebug() << "Using first model:" << modelList.first().modelName;
                }
            } else {
                qDebug() << "Failed to load model list:" << response.message;
                modelNameBtn->setText("加载失败");
            }
        },
        [this](const QString& error) {
            qDebug() << "Network error when loading model list:" << error;
            modelNameBtn->setText("加载失败");
        }
    );
}

void RightPanel::showModelPopupMenu() {
    if (modelList.isEmpty()) {
        qDebug() << "Model list is empty, cannot show menu";
        return;
    }
    
    qDebug() << "Showing model popup menu with" << modelList.size() << "items";
    
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
    
    for (const ModelInfo& model : modelList) {
        QAction* action = menu.addAction(model.modelName);
        action->setData(model.id);
        
        if (model.id == currentModel.id) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }
        
        connect(action, &QAction::triggered, this, &RightPanel::onModelSelected);
    }
    
    QPoint pos = modelContainer->mapToGlobal(QPoint(0, modelContainer->height()));
    menu.exec(pos);
}

void RightPanel::onModelSelected() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    QString modelId = action->data().toString();
    
    for (const ModelInfo& model : modelList) {
        if (model.id == modelId) {
            updateCurrentModel(model);
            break;
        }
    }
}

void RightPanel::updateCurrentModel(const ModelInfo& model) {
    currentModel = model;
    modelNameBtn->setText(model.modelName);
    
    TokenManager::instance().setValue(Constants::SELECTED_MODEL_ID_KEY, model.id);
    
    qDebug() << "Model selected:" << model.modelName << model.id;
}

void RightPanel::onModelMenuClicked() {
    if (isEditingPrompt) return;  // 编辑状态下不能点击
    
    qDebug() << "Model menu clicked, model list size:" << modelList.size();
    
    if (modelList.isEmpty()) {
        qDebug() << "Model list is empty, reloading...";
        loadModelList();
        return;
    }
    
    showModelPopupMenu();
}

void RightPanel::onDeepThinkToggled() {
    if (isEditingPrompt) {
        deepThinkBtn->setChecked(!deepThinkBtn->isChecked());
        return;
    }
    
    isDeepThinkSelected = deepThinkBtn->isChecked();
    qDebug() << "Deep think selected:" << isDeepThinkSelected;
    updateButtonsStyle();
}

void RightPanel::onLanguageToggle() {
    if (isEditingPrompt) return;  // 编辑状态下不能点击
    
    if (currentLanguage == "zh") {
        currentLanguage = "en";
        languageBtn->setText("English");
    } else {
        currentLanguage = "zh";
        languageBtn->setText("中文");
    }
    qDebug() << "Language changed to:" << currentLanguage;
}

void RightPanel::onSearchDocToggled() {
    if (isEditingPrompt) {
        searchDocBtn->setChecked(!searchDocBtn->isChecked());
        return;
    }
    
    isSearchDocSelected = searchDocBtn->isChecked();
    
    if (isSearchDocSelected) {
        docSelectionBtn->setVisible(true);
        isDocSelectionVisible = true;
        docSelectionBtn->setEnabled(true);
    } else {
        docSelectionBtn->setVisible(false);
        isDocSelectionVisible = false;
        docSelectionBtn->setChecked(false);
    }
    
    qDebug() << "Search doc selected:" << isSearchDocSelected
             << "Doc selection visible:" << isDocSelectionVisible;
    
    updateButtonsStyle();
}

void RightPanel::onDocSelectionToggled() {
    if (isEditingPrompt) {
        docSelectionBtn->setChecked(!docSelectionBtn->isChecked());
        return;
    }
    
    qDebug() << "Doc selection toggled:" << docSelectionBtn->isChecked();
}

void RightPanel::onEditPromptClicked() {
    if (isEditingPrompt) {
        // 退出编辑模式
        exitEditMode();
    } else {
        // 进入编辑模式
        enterEditMode();
    }
    
    onInputTextChanged();
}

void RightPanel::onSendClicked() {
    QString message = inputEdit->toPlainText().trimmed();
    if (message.isEmpty() || isReceivingMessage || isEditingPrompt) {
        return;
    }
    
    qDebug() << "Send button clicked, message:" << message.left(50) << "...";
    qDebug() << "Current model:" << currentModel.modelName;
    qDebug() << "Deep think:" << isDeepThinkSelected;
    qDebug() << "Search doc:" << isSearchDocSelected;
    
    // 连接WebSocket并发送消息
    connectWebSocket();
}

void RightPanel::updateButtonsStyle() {
    // 更新深度思考按钮样式
    if (isDeepThinkSelected) {
        deepThinkBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "   padding: 0 %4px;"
            "}"
        ).arg(Colors::PRIMARY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    } else {
        deepThinkBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: white;"
            "   color: %1;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "   padding: 0 %4px;"
            "}"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    }
    
    // 更新查询文档按钮样式
    if (isSearchDocSelected) {
        searchDocBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "   padding: 0 %4px;"
            "}"
        ).arg(Colors::PRIMARY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    } else {
        searchDocBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: white;"
            "   color: %1;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "   padding: 0 %4px;"
            "}"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    }
    
    // 更新文档选择按钮样式
    if (docSelectionBtn->isChecked()) {
        docSelectionBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "   padding: 0 %4px;"
            "}"
        ).arg(Colors::PRIMARY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    } else {
        docSelectionBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: white;"
            "   color: %1;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "   padding: 0 %4px;"
            "}"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    }
}

// 在 RightPanel.cpp 中添加
void RightPanel::loadChatHistory(const ChatHistory& chat) {
    qDebug() << "Loading chat history:" << chat.id << chat.prompt;
    
    // 清空当前消息
    clearAllMessages();
    
    // 显示消息区域，隐藏logo
    messageScrollArea->show();
    logoContainer->hide();
    
    // 设置当前 chatId
    currentChatId = chat.chatId;  // 使用历史对话的 chatId
    qDebug() << "Set current chat ID to:" << currentChatId;
    
    // 添加用户消息（prompt）
    if (!chat.prompt.isEmpty()) {
        addUserMessage(chat.prompt);
    }
    
    // 添加助手消息（如果有思考内容和响应内容）
    if (!chat.thinkContent.isEmpty() || !chat.responseContent.isEmpty()) {
        // 创建新的消息块
        MessageBlock block;
        block.id = ++currentMessageId;
        block.isThinking = false;
        block.thinkContent = chat.thinkContent;
        block.responseContent = chat.responseContent;
        
        QWidget* messageWidget = new QWidget(messageContainer);
        messageWidget->setObjectName(QString("assistant_msg_%1").arg(block.id));

        QHBoxLayout* layout = new QHBoxLayout(messageWidget);
        layout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
        layout->setSpacing(Dimens::PAGE_PADDING);
        layout->setAlignment(Qt::AlignTop);

        // 助手头像
        QLabel* avatarLabel = new QLabel(messageWidget);
        avatarLabel->setFixedSize(Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
        avatarLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QPixmap pixmap(Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, Dimens::SMALL_AVATAR, Dimens::SMALL_AVATAR);
        painter.fillPath(path, Colors::GRAY_COLOR);
        painter.setPen(Qt::white);
        QFont font;
        font.setPixelSize(Dimens::SMALL_AVATAR * 0.5);
        painter.setFont(font);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, "AI");
        avatarLabel->setPixmap(pixmap);

        // 消息内容容器
        QWidget* contentWidget = new QWidget(messageWidget);
        QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(Dimens::SMALL_MARGIN);

        // 思考内容标签
        QLabel* thinkLabel = new QLabel(contentWidget);
        thinkLabel->setWordWrap(true);
        thinkLabel->setStyleSheet(QString(
            "color: %1;"
            "font-size: %2px;"
            "background-color: %3;"
            "border-radius: %4px;"
            "padding: %5px;"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Colors::WHITE_COLOR.name())
         .arg(Dimens::SMALL_MARGIN)
         .arg(Dimens::PAGE_PADDING));
        thinkLabel->setMaximumWidth(QWIDGETSIZE_MAX);
        
        if (!chat.thinkContent.isEmpty()) {
            thinkLabel->setText(chat.thinkContent);
            thinkLabel->show();
        } else {
            thinkLabel->hide();
        }

        // 响应内容标签
        QLabel* responseLabel = new QLabel(contentWidget);
        responseLabel->setWordWrap(true);
        responseLabel->setStyleSheet(QString(
            "color: %1;"
            "font-size: %2px;"
            "background-color: %3;"
            "border-radius: %4px;"
            "padding: %5px;"
        ).arg(Colors::TEXT_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Colors::WHITE_COLOR.name())
         .arg(Dimens::SMALL_MARGIN)
         .arg(Dimens::PAGE_PADDING));
        responseLabel->setMaximumWidth(QWIDGETSIZE_MAX);
        responseLabel->setText(chat.responseContent);

        contentLayout->addWidget(thinkLabel);
        contentLayout->addWidget(responseLabel);

        layout->addWidget(avatarLabel);
        layout->addWidget(contentWidget, 1);

        messageLayout->insertWidget(messageLayout->count() - 1, messageWidget);
    }
    
    // 滚动到底部
    QTimer::singleShot(100, this, [this]() {
        messageScrollArea->verticalScrollBar()->setValue(
            messageScrollArea->verticalScrollBar()->maximum()
        );
    });
}

RightPanel::~RightPanel() {
    if (webSocket) {
        webSocket->close();
        delete webSocket;
    }
}