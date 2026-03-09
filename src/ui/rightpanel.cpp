#include "RightPanel.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include <QDebug>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>
#include <QJsonArray>
#include <QCursor>
#include <QMenu>
#include <QMouseEvent>  // 添加这个头文件

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
    , isDeepThinkSelected(false)
    , isSearchDocSelected(false)
    , isDocSelectionVisible(false)
    , currentLanguage("zh")  // 默认中文
{
    setStyleSheet("background-color: white;");  // 设置背景色为白色
    setupUI();
    
    // 加载模型列表
    loadModelList();
}

void RightPanel::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Dimens::PAGE_PADDING * 2, 
                                   Dimens::PAGE_PADDING * 2, 
                                   Dimens::PAGE_PADDING * 2, 
                                   Dimens::PAGE_PADDING * 2);
    mainLayout->setSpacing(Dimens::PAGE_PADDING * 2);
    
    // 添加顶部弹性空间，使内容垂直居中
    mainLayout->addStretch();
    
    // ========== Logo和欢迎语区域 ==========
    logoLayout = new QHBoxLayout();
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // Logo
    logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/images/logo.png");
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaled(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR,
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // 如果没有图片，显示文字占位
        logoLabel->setText("📋");
        logoLabel->setStyleSheet(QString("font-size: %1px; color: %2;")
                                .arg(Dimens::MIDDLE_AVATAR)
                                .arg(Colors::PRIMARY_COLOR.name()));
    }
    logoLabel->setFixedSize(Dimens::MIDDLE_AVATAR, Dimens::MIDDLE_AVATAR);
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // 欢迎语
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
    
    // ========== 输入框容器（包含输入框和按钮）==========
    inputContainer = new QWidget(this);
    inputContainer->setFixedWidth(900);  // 固定宽度900
    inputContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);  // 高度根据内容撑开
    inputContainer->setStyleSheet(QString(
        "QWidget {"
        "   background-color: white;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::DISABLE_COLOR.name())
     .arg(Dimens::BIG_BORDER_RADIUS));
    
    containerLayout = new QVBoxLayout(inputContainer);
    containerLayout->setContentsMargins(Dimens::PAGE_PADDING, 
                                        Dimens::PAGE_PADDING, 
                                        Dimens::PAGE_PADDING, 
                                        Dimens::PAGE_PADDING);
    containerLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 输入框
    inputEdit = new QTextEdit(inputContainer);
    inputEdit->setPlaceholderText("给chat发送消息");
    inputEdit->setFrameStyle(QFrame::NoFrame);  // 无边框
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
     .arg(Colors::DISABLE_COLOR.name())
     .arg(Colors::DISABLE_COLOR.name())
     .arg(Colors::PRIMARY_COLOR.name()));
    
    // ========== 顶部按钮区域（模型选择和语言切换）==========
    QWidget* topButtonContainer = new QWidget(inputContainer);
    topButtonContainer->setStyleSheet("background-color: transparent; border: none;");
    
    QHBoxLayout* topButtonLayout = new QHBoxLayout(topButtonContainer);
    topButtonLayout->setContentsMargins(0, 0, 0, 0);
    topButtonLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // ========== 模型选择容器（包含模型名称和下拉箭头）==========
    modelContainer = new QWidget(topButtonContainer);
    modelContainer->setCursor(Qt::PointingHandCursor);
    modelContainer->setStyleSheet("background-color: transparent; border: none;");
    
    modelLayout = new QHBoxLayout(modelContainer);
    modelLayout->setContentsMargins(0, 0, 0, 0);
    modelLayout->setSpacing(Dimens::SMALL_MARGIN);
    
    // 模型名称按钮
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
    
    // 下拉箭头按钮
    modelArrowBtn = new QPushButton(modelContainer);
    modelArrowBtn->setCursor(Qt::PointingHandCursor);
    modelArrowBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    
    // 加载下拉箭头图标
    QPixmap arrowPixmap(":/images/icon_down.png");
    if (!arrowPixmap.isNull()) {
        modelArrowBtn->setIcon(QIcon(arrowPixmap));
        modelArrowBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
        modelArrowBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    } else {
        // 如果没有图标，使用文字代替
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
    
    // 连接模型选择信号（使用按钮的clicked信号，而不是mousePressEvent）
    connect(modelNameBtn, &QPushButton::clicked, this, &RightPanel::onModelMenuClicked);
    connect(modelArrowBtn, &QPushButton::clicked, this, &RightPanel::onModelMenuClicked);
    
    // 中英文切换按钮（无边框，带图标，右对齐）
    languageBtn = new QPushButton("中文", topButtonContainer);
    languageBtn->setCursor(Qt::PointingHandCursor);
    languageBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    languageBtn->setMinimumWidth(100);
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
    
    // 加载切换图标
    QPixmap switchPixmap(":/images/icon_switch.png");
    if (!switchPixmap.isNull()) {
        languageBtn->setIcon(QIcon(switchPixmap));
        languageBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
    } else {
        // 如果没有图标，使用文字代替
        qDebug() << "Switch icon not found, using text fallback";
        languageBtn->setText("中文 🔄");
    }
    
    // 设置图标在文字右侧
    languageBtn->setLayoutDirection(Qt::RightToLeft);
    
    // 添加到顶部按钮布局
    topButtonLayout->addWidget(modelContainer);
    topButtonLayout->addStretch();  // 添加弹性空间，将语言按钮推到右边
    topButtonLayout->addWidget(languageBtn);
    
    // ========== 底部按钮区域 ==========
    buttonContainer = new QWidget(inputContainer);
    buttonContainer->setStyleSheet("background-color: transparent; border: none;");
    
    buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(Dimens::PAGE_PADDING);
    
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
    ).arg(Colors::DISABLE_COLOR.name())
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
    ).arg(Colors::DISABLE_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    // 文档选择按钮（初始隐藏）
    docSelectionBtn = new QPushButton("选择文档", buttonContainer);
    docSelectionBtn->setCursor(Qt::PointingHandCursor);
    docSelectionBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    docSelectionBtn->setMinimumWidth(100);
    docSelectionBtn->setVisible(false);  // 初始隐藏
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
    ).arg(Colors::DISABLE_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Dimens::PAGE_PADDING)
     .arg(Colors::PRIMARY_COLOR.name()));
    
    // 连接信号
    connect(deepThinkBtn, &QPushButton::toggled, this, &RightPanel::onDeepThinkToggled);
    connect(languageBtn, &QPushButton::clicked, this, &RightPanel::onLanguageToggle);
    connect(searchDocBtn, &QPushButton::toggled, this, &RightPanel::onSearchDocToggled);
    connect(docSelectionBtn, &QPushButton::toggled, this, &RightPanel::onDocSelectionToggled);
    
    // 将按钮添加到按钮布局
    buttonLayout->addWidget(deepThinkBtn);
    buttonLayout->addWidget(searchDocBtn);
    buttonLayout->addWidget(docSelectionBtn);
    buttonLayout->addStretch();  // 添加弹性空间
    
    // 将所有组件添加到容器布局
    containerLayout->addWidget(topButtonContainer);
    containerLayout->addWidget(inputEdit);
    containerLayout->addWidget(buttonContainer);
    
    // 创建一个水平布局来容纳居中的inputContainer
    QHBoxLayout* centerInputLayout = new QHBoxLayout();
    centerInputLayout->addStretch();
    centerInputLayout->addWidget(inputContainer);
    centerInputLayout->addStretch();
    
    // 添加到主布局
    mainLayout->addLayout(logoLayout);
    mainLayout->addSpacing(Dimens::PAGE_PADDING * 2);
    mainLayout->addLayout(centerInputLayout);
    
    // 添加底部弹性空间，使内容垂直居中
    mainLayout->addStretch();
}

void RightPanel::loadModelList() {
    qDebug() << "Loading model list from:" << Constants::Endpoints::GET_MODEL_LIST;
    
    // 确保 token 已设置
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
                
                // 从缓存中获取上次保存的模型ID
                QString cachedModelId = TokenManager::instance().getValue(Constants::SELECTED_MODEL_ID_KEY).toString();
                qDebug() << "Cached model ID:" << cachedModelId;
                
                bool found = false;
                
                // 检查缓存的模型是否在列表中
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
                
                // 如果没找到，选择第一条
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
        
        // 标记当前选中的模型
        if (model.id == currentModel.id) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }
        
        connect(action, &QAction::triggered, this, &RightPanel::onModelSelected);
    }
    
    // 显示菜单
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
    
    // 保存到缓存
    TokenManager::instance().setValue(Constants::SELECTED_MODEL_ID_KEY, model.id);
    
    qDebug() << "Model selected:" << model.modelName << model.id;
}

void RightPanel::onModelMenuClicked() {
    qDebug() << "Model menu clicked, model list size:" << modelList.size();
    
    if (modelList.isEmpty()) {
        qDebug() << "Model list is empty, reloading...";
        loadModelList();  // 尝试重新加载模型列表
        return;
    }
    
    showModelPopupMenu();
}

void RightPanel::onDeepThinkToggled() {
    isDeepThinkSelected = deepThinkBtn->isChecked();
    qDebug() << "Deep think selected:" << isDeepThinkSelected;
    updateButtonsStyle();
}

void RightPanel::onLanguageToggle() {
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
    isSearchDocSelected = searchDocBtn->isChecked();
    
    // 联动显示/隐藏文档选择按钮
    if (isSearchDocSelected) {
        // 查询文档选中时，显示文档选择按钮
        docSelectionBtn->setVisible(true);
        isDocSelectionVisible = true;
        
        // 可选：添加一个淡入动画效果
        docSelectionBtn->setEnabled(true);
    } else {
        // 查询文档未选中时，隐藏文档选择按钮
        docSelectionBtn->setVisible(false);
        isDocSelectionVisible = false;
        docSelectionBtn->setChecked(false);  // 重置选中状态
    }
    
    qDebug() << "Search doc selected:" << isSearchDocSelected
             << "Doc selection visible:" << isDocSelectionVisible;
    
    updateButtonsStyle();
}

void RightPanel::onDocSelectionToggled() {
    qDebug() << "Doc selection toggled:" << docSelectionBtn->isChecked();
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
        ).arg(Colors::DISABLE_COLOR.name())
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
        ).arg(Colors::DISABLE_COLOR.name())
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
        ).arg(Colors::DISABLE_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING));
    }
}