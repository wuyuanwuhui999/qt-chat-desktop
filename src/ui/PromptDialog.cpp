#include "PromptDialog.h"
#include "PromptFormDialog.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include <QFrame>
#include <QScrollBar>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QPainter>
#include <QPointer>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>

namespace {
    const int kDialogWidth  = 760;
    const int kDialogHeight = 640;
    const int kShadowMargin = 16;          // 无边框窗口四周留给阴影的空白
    const int kScrollBarWidth = 8;
    const int kUseButtonWidth = 90;
    const int kScrollLoadThreshold = 40;   // 距底部多少像素触发加载更多

    // 生成带透明度的图标（规范：图标透明度 0.5）
    QIcon transparentIcon(const QString& path, double opacity = 0.5) {
        QPixmap src(path);
        if (src.isNull()) return QIcon();
        QPixmap out(src.size());
        out.fill(Qt::transparent);
        QPainter painter(&out);
        painter.setOpacity(opacity);
        painter.drawPixmap(0, 0, src);
        painter.end();
        return QIcon(out);
    }

    // 把图标的形状重新着色后再加透明。
    // icon_close.png 本身是纯白色（原用途是配彩色底），直接画在白色卡片上会看不见。
    QIcon tintedIcon(const QString& path, const QColor& color, double opacity = 0.5) {
        QPixmap src(path);
        if (src.isNull()) return QIcon();
        QPixmap out(src.size());
        out.fill(Qt::transparent);
        QPainter painter(&out);
        painter.setOpacity(opacity);
        painter.drawPixmap(0, 0, src);
        // 保留原有 alpha，把颜色替换成目标色
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(out.rect(), color);
        painter.end();
        return QIcon(out);
    }

    // 文本框样式（用于条目上的操作按钮）
    QString itemButtonStyle(const QColor& borderColor,
                            const QColor& textColor,
                            const QColor& backgroundColor) {
        return QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: 1px solid %3;"
            "   border-radius: %4px;"
            "   font-size: %5px;"
            "}"
        ).arg(backgroundColor.name())
         .arg(textColor.name())
         .arg(borderColor.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL);
    }

    // 按宽度折行，最多 maxLines 行，超出用省略号收尾
    QString elideToLines(const QString& text, const QFont& font, int width, int maxLines) {
        if (width <= 0 || text.isEmpty() || maxLines <= 0) return text;

        const QFontMetrics fm(font);
        QStringList lines;
        QString current;
        bool truncated = false;

        for (int i = 0; i < text.size(); ++i) {
            const QChar ch = text.at(i);
            if (ch == QLatin1Char('\n')) {
                lines.append(current);
                current.clear();
                if (lines.size() >= maxLines) {
                    truncated = (i + 1 < text.size());
                    break;
                }
                continue;
            }
            const QString candidate = current + ch;
            if (fm.horizontalAdvance(candidate) > width && !current.isEmpty()) {
                lines.append(current);
                current = QString(ch);
                if (lines.size() >= maxLines) {
                    truncated = true;
                    break;
                }
            } else {
                current = candidate;
            }
        }

        if (!truncated) {
            if (!current.isEmpty()) lines.append(current);
            if (lines.size() <= maxLines) return lines.join(QLatin1Char('\n'));
        }

        while (lines.size() > maxLines) lines.removeLast();
        if (lines.isEmpty()) return text;

        QString last = lines.last();
        while (!last.isEmpty() && fm.horizontalAdvance(last + QStringLiteral("…")) > width) {
            last.chop(1);
        }
        lines.last() = last + QStringLiteral("…");
        return lines.join(QLatin1Char('\n'));
    }
}

PromptDialog::PromptDialog(const QString& tenantId,
                           const QString& currentPromptId,
                           QWidget* parent)
    : QDialog(parent)
    , m_tenantId(tenantId)
    , m_usedPromptId(currentPromptId)
    , m_pageNum(1)
    , m_pageSize(20)
    , m_total(0)
    , m_isLoading(false)
    , m_hasMore(true)
    , m_root(nullptr)
    , m_titleBar(nullptr)
    , m_titleBarTitle(nullptr)
    , m_refreshBtn(nullptr)
    , m_addBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_dragging(false)
{
    setWindowTitle("提示词");
    // 无边框：标题与刷新/新增图标都画在自绘标题栏里，四周留出阴影空间
    setFixedSize(kDialogWidth + kShadowMargin * 2, kDialogHeight + kShadowMargin * 2);
    setModal(true);

    // 条目文本宽度：对话框宽度减去各级间距、三个操作按钮与滚动条
    m_textWidth = kDialogWidth
                - Dimens::PAGE_PADDING * 8      // 对话框/内容区/列表卡片/条目 的左右边距
                - Dimens::BTN_HEIGHT * 2        // 删除、编辑两个图标按钮
                - kUseButtonWidth               // 使用按钮
                - Dimens::PAGE_PADDING * 3      // 行内三处间距
                - kScrollBarWidth;
    if (m_textWidth < 100) m_textWidth = 100;

    setupUI();
    loadPrompts(1, false);
}

void PromptDialog::setupUI()
{
    // 无边框窗口 + 阴影：这样才能把刷新/新增图标放进标题栏右侧
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(kShadowMargin, kShadowMargin, kShadowMargin, kShadowMargin);
    outerLayout->setSpacing(0);

    m_root = new QWidget(this);
    m_root->setObjectName("promptDialogRoot");
    m_root->setStyleSheet(QString(
        "QWidget#promptDialogRoot {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    // 用主题文字色加透明度作为阴影颜色，避免硬编码颜色
    QColor shadowColor = Colors::TEXT_COLOR;
    shadowColor.setAlpha(70);
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(m_root);
    shadow->setBlurRadius(kShadowMargin);
    shadow->setOffset(0, 2);
    shadow->setColor(shadowColor);
    m_root->setGraphicsEffect(shadow);

    outerLayout->addWidget(m_root);

    m_mainLayout = new QVBoxLayout(m_root);
    m_mainLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                     Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    m_mainLayout->setSpacing(Dimens::PAGE_PADDING);

    m_mainLayout->addWidget(createHeaderArea());
    m_mainLayout->addWidget(createContentArea(), 1);

    m_searchDebounce = new QTimer(this);
    m_searchDebounce->setSingleShot(true);
    m_searchDebounce->setInterval(300);   // 输入防抖，避免每个字符都打一次接口
    connect(m_searchDebounce, &QTimer::timeout, this, [this]() {
        loadPrompts(1, false);
    });
}

QWidget* PromptDialog::createHeaderArea()
{
    // 自绘标题栏：标题在左，刷新 / 新增 / 关闭在右；按住这条可拖动窗口
    m_titleBar = new QWidget(m_root);
    m_titleBar->setObjectName("promptDialogTitleBar");
    m_titleBar->setStyleSheet("background-color: transparent;");
    m_titleBar->installEventFilter(this);

    QHBoxLayout* layout = new QHBoxLayout(m_titleBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Dimens::PAGE_PADDING);

    // 标题
    m_titleBarTitle = new QLabel("提示词", m_titleBar);
    m_titleBarTitle->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "font-weight: bold;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_BIG));

    // 刷新图标
    m_refreshBtn = new QPushButton(m_titleBar);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    m_refreshBtn->setToolTip("刷新");
    m_refreshBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    m_refreshBtn->setIcon(transparentIcon(":/images/icon_refresh.png"));
    m_refreshBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));

    // 新增图标
    m_addBtn = new QPushButton(m_titleBar);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    m_addBtn->setToolTip("新增提示词");
    m_addBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    m_addBtn->setIcon(transparentIcon(":/images/icon_add.png"));
    m_addBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));

    // 关闭图标（无边框窗口没有系统关闭按钮，需要自己画一个）
    m_closeBtn = new QPushButton(m_titleBar);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    m_closeBtn->setToolTip("关闭");
    m_closeBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    m_closeBtn->setIcon(tintedIcon(":/images/icon_close.png", Colors::SUB_TITLE_COLOR));
    m_closeBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));

    layout->addWidget(m_titleBarTitle);
    layout->addStretch();
    layout->addWidget(m_refreshBtn);
    layout->addWidget(m_addBtn);
    layout->addWidget(m_closeBtn);

    connect(m_refreshBtn, &QPushButton::clicked, this, &PromptDialog::onRefreshClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &PromptDialog::onAddClicked);
    // 关闭等同取消
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    // 标题栏与内容区之间的分隔线
    QWidget* wrapper = new QWidget(m_root);
    wrapper->setStyleSheet("background-color: transparent;");
    QVBoxLayout* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(Dimens::PAGE_PADDING);

    QFrame* line = new QFrame(wrapper);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet(
        QString("background-color: %1; border: none; max-height: 1px; min-height: 1px;")
            .arg(Colors::GRAY_COLOR.name()));

    wrapperLayout->addWidget(m_titleBar);
    wrapperLayout->addWidget(line);
    return wrapper;
}

bool PromptDialog::eventFilter(QObject* watched, QEvent* event)
{
    // 无边框窗口只能自己实现拖动：按住标题栏移动窗口
    if (watched == m_titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragOffset = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
                move(mouseEvent->globalPosition().toPoint() - m_dragOffset);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

QWidget* PromptDialog::createContentArea()
{
    // 内容区：灰底，标准内间距
    m_contentWidget = new QWidget(m_root);
    m_contentWidget->setObjectName("promptDialogContent");
    m_contentWidget->setStyleSheet(QString(
        "QWidget#promptDialogContent {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::BACKGROUND_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                        Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    m_contentLayout->setSpacing(Dimens::PAGE_PADDING);

    // 搜索框（单行：INPUT_HEIGHT，圆角 INPUT_HEIGHT/2）
    m_searchEdit = new QLineEdit(m_contentWidget);
    m_searchEdit->setPlaceholderText("搜索提示词");
    m_searchEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    m_searchEdit->setStyleSheet(QString(
        "QLineEdit {"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: 0 %3px;"
        "   font-size: %4px;"
        "   background-color: %5;"
        "}"
        "QLineEdit:focus {"
        "   border-color: %6;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::INPUT_HEIGHT / 2)
     .arg(Dimens::PAGE_PADDING)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::WHITE_COLOR.name())
     .arg(Colors::PRIMARY_COLOR.name()));
    m_contentLayout->addWidget(m_searchEdit);

    // 列表卡片
    m_listCard = new QWidget(m_contentWidget);
    m_listCard->setObjectName("promptListCard");
    m_listCard->setStyleSheet(QString(
        "QWidget#promptListCard {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    QVBoxLayout* cardLayout = new QVBoxLayout(m_listCard);
    cardLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    cardLayout->setSpacing(Dimens::PAGE_PADDING);

    m_scrollArea = new QScrollArea(m_listCard);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    m_scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "   background-color: transparent;"
        "   width: " + QString::number(kScrollBarWidth) + "px;"
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

    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background-color: transparent;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);   // 条目之间的分隔由每条自带的灰色横线负责
    m_listLayout->addStretch();
    m_scrollArea->setWidget(m_listContainer);

    cardLayout->addWidget(m_scrollArea);
    m_contentLayout->addWidget(m_listCard, 1);

    // 取消 / 确定
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(Dimens::PAGE_PADDING);

    m_confirmBtn = new QPushButton("确定", m_contentWidget);
    m_confirmBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_cancelBtn = new QPushButton("取消", m_contentWidget);
    m_cancelBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 取消按钮：背景透明，边框和文字为 GRAY_COLOR
    m_cancelBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: %1;"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "}"
        "QPushButton:hover {"
        "   border-color: %4;"
        "   color: %4;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.name()));

    buttonLayout->addWidget(m_confirmBtn);
    buttonLayout->addWidget(m_cancelBtn);
    m_contentLayout->addLayout(buttonLayout);

    connect(m_confirmBtn, &QPushButton::clicked, this, &PromptDialog::onConfirmClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PromptDialog::onSearchTextChanged);
    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &PromptDialog::onScrollValueChanged);

    updateConfirmButtonState();
    return m_contentWidget;
}

void PromptDialog::onSearchTextChanged()
{
    if (m_searchDebounce) {
        m_searchDebounce->start();
    }
}

void PromptDialog::onRefreshClicked()
{
    loadPrompts(1, false);
}

void PromptDialog::onAddClicked()
{
    openAddDialog();
}

void PromptDialog::onScrollValueChanged(int value)
{
    QScrollBar* bar = m_scrollArea->verticalScrollBar();
    if (bar->maximum() <= 0) return;
    if (m_hasMore && !m_isLoading && value >= bar->maximum() - kScrollLoadThreshold) {
        loadPrompts(m_pageNum + 1, true);
    }
}

void PromptDialog::loadPrompts(int pageNum, bool append)
{
    if (m_isLoading) return;
    m_isLoading = true;

    const QString endpoint = Constants::withQuery(Constants::Endpoints::GET_PROMPT_LIST, {
        {"tenantId", m_tenantId},
        {"keyword", m_searchEdit ? m_searchEdit->text().trimmed() : QString()},
        {"pageSize", QString::number(m_pageSize)},
        {"pageNum", QString::number(pageNum)}
    });
    qDebug() << "Loading prompt list from:" << endpoint;

    QPointer<PromptDialog> self(this);
    NetworkManager::instance().get(
        endpoint,
        [this, self, pageNum, append](const ApiResponse& response) {
            if (!self) return;
            m_isLoading = false;

            if (!response.isSuccess()) {
                qDebug() << "Failed to load prompt list:" << response.message;
                QMessageBox::warning(this, "提示",
                    "加载提示词失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
                return;
            }

            m_pageNum = pageNum;
            m_total = response.total;
            m_hasMore = (m_pageSize * pageNum) < m_total;

            const QJsonArray array = response.data.toJsonArray();
            qDebug() << "Prompt array size:" << array.size() << "total:" << m_total;

            if (!append) {
                clearList();
            }

            for (const QJsonValue& value : array) {
                const Prompt prompt = Prompt::fromJson(value.toObject());
                if (!prompt.isValid()) continue;
                m_prompts.append(prompt);
                addPromptItem(prompt);
            }

            if (!append && m_prompts.isEmpty()) {
                showEmptyHint("暂无提示词");
            }

            updateSeparators();
            applyUsedState();
            updateConfirmButtonState();
        },
        [this, self](const QString& error) {
            if (!self) return;
            m_isLoading = false;
            qDebug() << "Network error when loading prompt list:" << error;
            QMessageBox::warning(this, "提示", "加载提示词失败：" + error);
        }
    );
}

void PromptDialog::addPromptItem(const Prompt& prompt)
{
    QWidget* item = new QWidget(m_listContainer);
    item->setStyleSheet("background-color: transparent;");

    QVBoxLayout* itemLayout = new QVBoxLayout(item);
    itemLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    itemLayout->setSpacing(Dimens::PAGE_PADDING);

    QHBoxLayout* row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(Dimens::PAGE_PADDING);

    // 提示词文本：最多三行，超出省略号
    QLabel* textLabel = new QLabel(item);
    textLabel->setWordWrap(true);
    textLabel->setFixedWidth(m_textWidth);
    textLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    textLabel->setText(elideToLines(prompt.prompt, textLabel->font(), m_textWidth, 3));

    // 删除
    QPushButton* deleteBtn = new QPushButton(item);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    deleteBtn->setToolTip("删除");
    deleteBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    deleteBtn->setIcon(tintedIcon(":/images/icon_close.png", Colors::WARN_COLOR));
    deleteBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));

    // 编辑
    QPushButton* editBtn = new QPushButton(item);
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    editBtn->setToolTip("编辑");
    editBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    editBtn->setIcon(transparentIcon(":/images/icon_edit.png"));
    editBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));

    // 使用
    QPushButton* useBtn = new QPushButton("使用", item);
    useBtn->setCursor(Qt::PointingHandCursor);
    useBtn->setFixedSize(kUseButtonWidth, Dimens::BTN_HEIGHT);

    row->addWidget(textLabel);
    row->addStretch();
    row->addWidget(deleteBtn);
    row->addWidget(editBtn);
    row->addWidget(useBtn);
    itemLayout->addLayout(row);

    // 条目之间的灰色横线
    QFrame* separator = new QFrame(item);
    separator->setObjectName("itemSeparator");
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet(
        QString("background-color: %1; border: none; max-height: 1px; min-height: 1px;")
            .arg(Colors::GRAY_COLOR.name()));
    itemLayout->addWidget(separator);

    // 数据绑定：把控件存到属性上，便于 applyUsedState 统一刷新
    item->setProperty("promptId", prompt.id);
    item->setProperty("textLabel", QVariant::fromValue<QObject*>(textLabel));
    item->setProperty("useBtn", QVariant::fromValue<QObject*>(useBtn));
    m_itemWidgets.insert(prompt.id, item);

    connect(deleteBtn, &QPushButton::clicked, this, [this, prompt]() {
        deletePrompt(prompt);
    });
    connect(editBtn, &QPushButton::clicked, this, [this, prompt]() {
        openEditDialog(prompt);
    });
    connect(useBtn, &QPushButton::clicked, this, [this, prompt]() {
        toggleUse(prompt);
    });

    m_listLayout->insertWidget(m_listLayout->count() - 1, item);
}

void PromptDialog::clearList()
{
    m_prompts.clear();
    m_itemWidgets.clear();

    while (m_listLayout->count() > 1) {
        QLayoutItem* item = m_listLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void PromptDialog::showEmptyHint(const QString& text)
{
    QLabel* label = new QLabel(text, m_listContainer);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    m_listLayout->insertWidget(m_listLayout->count() - 1, label);
}

void PromptDialog::removePromptItem(const QString& promptId)
{
    QWidget* item = m_itemWidgets.take(promptId);
    if (item) {
        m_listLayout->removeWidget(item);
        item->deleteLater();
    }

    for (int i = 0; i < m_prompts.size(); ++i) {
        if (m_prompts[i].id == promptId) {
            m_prompts.removeAt(i);
            break;
        }
    }

    if (m_total > 0) --m_total;
    m_hasMore = (m_pageSize * m_pageNum) < m_total;

    if (m_prompts.isEmpty()) {
        showEmptyHint("暂无提示词");
    }

    updateSeparators();
    updateConfirmButtonState();
}

void PromptDialog::updateSeparators()
{
    QList<QWidget*> items;
    for (int i = 0; i < m_listLayout->count(); ++i) {
        QLayoutItem* layoutItem = m_listLayout->itemAt(i);
        if (layoutItem && layoutItem->widget()) {
            items.append(layoutItem->widget());
        }
    }

    // 最后一个条目的分割线不显示
    for (int i = 0; i < items.size(); ++i) {
        QFrame* separator = items[i]->findChild<QFrame*>("itemSeparator");
        if (separator) {
            separator->setVisible(i != items.size() - 1);
        }
    }
}

void PromptDialog::applyUsedState()
{
    for (auto it = m_itemWidgets.constBegin(); it != m_itemWidgets.constEnd(); ++it) {
        QWidget* item = it.value();
        const bool isUsed = (!m_usedPromptId.isEmpty() && it.key() == m_usedPromptId);

        QLabel* textLabel = qobject_cast<QLabel*>(item->property("textLabel").value<QObject*>());
        QPushButton* useBtn = qobject_cast<QPushButton*>(item->property("useBtn").value<QObject*>());
        if (!textLabel || !useBtn) continue;

        // 正在使用的提示词：文字用激活色
        textLabel->setStyleSheet(QString(
            "color: %1;"
            "font-size: %2px;"
            "background-color: transparent;"
        ).arg(isUsed ? Colors::PRIMARY_COLOR.name() : Colors::TEXT_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL));

        if (isUsed) {
            useBtn->setText("取消使用");
            useBtn->setStyleSheet(itemButtonStyle(Colors::PRIMARY_COLOR,
                                                  Colors::WHITE_COLOR,
                                                  Colors::PRIMARY_COLOR));
        } else {
            useBtn->setText("使用");
            useBtn->setStyleSheet(itemButtonStyle(Colors::PRIMARY_COLOR,
                                                  Colors::PRIMARY_COLOR,
                                                  Colors::WHITE_COLOR));
        }
    }
}

void PromptDialog::updateConfirmButtonState()
{
    // 必须使用一条提示词之后才能点确定
    const bool enabled = !m_usedPromptId.isEmpty();
    m_confirmBtn->setEnabled(enabled);

    if (enabled) {
        m_confirmBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: none;"
            "   border-radius: %3px;"
            "   font-size: %4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: %5;"
            "}"
        ).arg(Colors::PRIMARY_COLOR.name())
         .arg(Colors::WHITE_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Colors::PRIMARY_COLOR.lighter(110).name()));
    } else {
        m_confirmBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: none;"
            "   border-radius: %3px;"
            "   font-size: %4px;"
            "}"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Colors::WHITE_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL));
    }
}

void PromptDialog::toggleUse(const Prompt& prompt)
{
    if (m_usedPromptId == prompt.id) {
        m_usedPromptId.clear();
    } else {
        m_usedPromptId = prompt.id;
    }

    qDebug() << "Used prompt id:" << m_usedPromptId;

    applyUsedState();
    updateConfirmButtonState();
}

void PromptDialog::openAddDialog()
{
    PromptFormDialog dialog(PromptFormDialog::Add, m_tenantId, Prompt(), this);
    if (dialog.exec() == QDialog::Accepted) {
        // 需求：添加成功后关闭弹窗，由用户点刷新重新加载
        qDebug() << "Prompt added, click refresh to reload";
    }
}

void PromptDialog::openEditDialog(const Prompt& prompt)
{
    PromptFormDialog dialog(PromptFormDialog::Update, m_tenantId, prompt, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 需求：更新成功后关闭弹窗，由用户点刷新重新加载
        qDebug() << "Prompt updated, click refresh to reload";
    }
}

void PromptDialog::deletePrompt(const Prompt& prompt)
{
    const QMessageBox::StandardButton ret = QMessageBox::question(
        this, "提示", "确定要删除该提示词吗？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }

    const QString endpoint = QString(Constants::Endpoints::DELETE_PROMPT)
                                 .arg(prompt.id)
                                 .arg(m_tenantId);
    qDebug() << "Deleting prompt:" << endpoint;

    QPointer<PromptDialog> self(this);
    NetworkManager::instance().del(
        endpoint,
        [this, self, prompt](const ApiResponse& response) {
            if (!self) return;
            if (response.isSuccess() && response.data.toInt() > 0) {
                // 删除成功：从列表中移除
                if (m_usedPromptId == prompt.id) {
                    m_usedPromptId.clear();
                }
                removePromptItem(prompt.id);
                applyUsedState();
            } else {
                QMessageBox::warning(this, "提示",
                    "删除提示词失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
            }
        },
        [this, self](const QString& error) {
            if (!self) return;
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}

void PromptDialog::onConfirmClicked()
{
    if (m_usedPromptId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先使用一条提示词");
        return;
    }
    accept();
}
