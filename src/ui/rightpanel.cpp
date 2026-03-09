#include "RightPanel.h"
#include <QDebug>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
    , isDeepThinkSelected(false)
    , isSearchDocSelected(false)
    , isDocSelectionVisible(false)
    , currentLanguage("zh")  // 默认中文
{
    setStyleSheet("background-color: white;");  // 设置背景色为白色
    setupUI();
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
    
    // ========== 按钮容器 ==========
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
    
    // 中英文切换按钮（无边框，带图标，右对齐）
    languageBtn = new QPushButton("中文", buttonContainer);
    languageBtn->setCursor(Qt::PointingHandCursor);
    languageBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    languageBtn->setMinimumWidth(100);
    languageBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: %1;"
        "   border: none;"
        "   font-size: %3px;"
        "   padding: 0 %4px;"
        "   text-align: right;"
        "}"
    ).arg(Colors::TEXT_COLOR.name())  // 黑色文字
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
    languageBtn->setLayoutDirection(Qt::RightToLeft);  // 设置布局方向为从右到左，使图标在文字右侧
    
    // 连接信号
    connect(deepThinkBtn, &QPushButton::toggled, this, &RightPanel::onDeepThinkToggled);
    connect(languageBtn, &QPushButton::clicked, this, &RightPanel::onLanguageToggle);
    connect(searchDocBtn, &QPushButton::toggled, this, &RightPanel::onSearchDocToggled);
    connect(docSelectionBtn, &QPushButton::toggled, this, &RightPanel::onDocSelectionToggled);
    
    // 将按钮添加到按钮布局
    buttonLayout->addWidget(deepThinkBtn);
    buttonLayout->addWidget(searchDocBtn);
    buttonLayout->addWidget(docSelectionBtn);
    buttonLayout->addStretch();  // 添加弹性空间，将后面的按钮推到右边
    buttonLayout->addWidget(languageBtn);  // 语言按钮放在最右边
    
    // 将输入框和按钮容器添加到容器布局
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
    
    // 语言按钮样式保持不变（无边框，黑色文字）
    // 不需要更新，因为样式在setupUI中已设置且保持不变
}