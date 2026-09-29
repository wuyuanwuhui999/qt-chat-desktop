// DocumentDialog.cpp
#include "DocumentDialog.h"
#include "DocPermissionDialog.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPointer>

namespace {

    QString scrollAreaStyle() {
        return "QScrollArea { background-color: " + Colors::WHITE_COLOR.name() + "; border: none; }";
    }

    QString scrollBarStyle() {
        return "QScrollBar:vertical {"
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
               "}";
    }

    // 目录展开/收起的箭头：down=true 指向下（展开），false 指向右（收起）
    void applyArrowIcon(QPushButton* btn, bool down) {
        if (!btn) return;
        QPixmap arrowPixmap(":/images/icon_down.png");
        if (arrowPixmap.isNull()) return;

        QPixmap transparentPixmap(arrowPixmap.size());
        transparentPixmap.fill(Qt::transparent);
        {
            QPainter painter(&transparentPixmap);
            painter.setOpacity(0.5);   // 规范：图标透明度 0.5
            painter.drawPixmap(0, 0, arrowPixmap);
        }

        if (down) {
            btn->setIcon(QIcon(transparentPixmap));
        } else {
            QPixmap rotatedPixmap(transparentPixmap.size());
            rotatedPixmap.fill(Qt::transparent);
            QPainter rotatePainter(&rotatedPixmap);
            rotatePainter.translate(rotatedPixmap.width() / 2, rotatedPixmap.height() / 2);
            rotatePainter.rotate(-90);
            rotatePainter.translate(-rotatedPixmap.width() / 2, -rotatedPixmap.height() / 2);
            rotatePainter.drawPixmap(0, 0, transparentPixmap);
            btn->setIcon(QIcon(rotatedPixmap));
        }
        btn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
    }

    // "..." 图标：工程里没有对应的素材，用三个圆点画出来
    QIcon moreDotsIcon(const QColor& color, double opacity = 0.5) {
        const int s = Dimens::MIDDLE_ICON_SIZE;
        QPixmap pm(s, s);
        pm.fill(Qt::transparent);

        QPainter painter(&pm);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setOpacity(opacity);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);

        const double radius = s / 12.0;
        const double centerY = s / 2.0;
        for (int i = 0; i < 3; ++i) {
            const double centerX = s / 6.0 + i * (s / 3.0);
            painter.drawEllipse(QPointF(centerX, centerY), radius, radius);
        }
        painter.end();
        return QIcon(pm);
    }

    QString documentMenuStyle() {
        return QString(
            "QMenu {"
            "   background-color: %1;"
            "   border: 1px solid %2;"
            "   border-radius: %3px;"
            "   padding: %4px;"
            "}"
            "QMenu::item {"
            "   background-color: transparent;"
            "   color: %5;"
            "   padding: %4px %4px;"
            "   min-width: %6px;"
            "   min-height: %7px;"
            "   font-size: %8px;"
            "}"
            "QMenu::item:selected {"
            "   background-color: %9;"
            "   color: %10;"
            "}"
        ).arg(Colors::WHITE_COLOR.name())
         .arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::MODULE_BORDER_RADIUS)
         .arg(Dimens::PAGE_PADDING)
         .arg(Colors::TEXT_COLOR.name())
         .arg(Dimens::POPUP_MENU_WIDTH)
         .arg(Dimens::POPUP_MENU_HEIGHT)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Colors::PRIMARY_COLOR.name())
         .arg(Colors::WHITE_COLOR.name());
    }

}

DocumentDialog::DocumentDialog(const QString& tenantId, QWidget *parent)
    : QDialog(parent)
    , m_tenantId(tenantId)
    , m_publicLoaded(false)
{
    setWindowTitle("选择文档");
    setFixedSize(500, 500);
    setModal(true);
    
    setupUI();
    loadDirectoryList();
}

DocumentDialog::~DocumentDialog()
{
}

void DocumentDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 顶部页签：我的文档 / 公共文档
    m_mainLayout->addWidget(createTabBar());

    // 页签下方的分割线
    m_tabLine = new QFrame(this);
    m_tabLine->setFrameShape(QFrame::HLine);
    m_tabLine->setFrameShadow(QFrame::Plain);
    m_tabLine->setStyleSheet(QString("background-color: %1; border: none; max-height: 1px; min-height: 1px;").arg(Colors::GRAY_COLOR.name()));
    m_mainLayout->addWidget(m_tabLine);

    // 两个页签各自的页面
    m_pageStack = new QStackedWidget(this);
    m_myDocPage = createMyDocPage();
    m_publicDocPage = createPublicDocPage();
    m_pageStack->addWidget(m_myDocPage);
    m_pageStack->addWidget(m_publicDocPage);
    m_mainLayout->addWidget(m_pageStack, 1);

    // 默认选中「我的文档」
    switchToTab(0);

    // 底部按钮容器
    QWidget* buttonWidget = new QWidget(this);
    buttonWidget->setStyleSheet("background-color: " + Colors::WHITE_COLOR.name() + ";");
    
    QVBoxLayout* buttonWrapperLayout = new QVBoxLayout(buttonWidget);
    buttonWrapperLayout->setContentsMargins(0, 0, 0, Dimens::PAGE_PADDING);
    buttonWrapperLayout->setSpacing(0);
    
    // 添加顶部分割线 - 颜色为 Colors::GRAY_COLOR
    QFrame* topLine = new QFrame(buttonWidget);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Plain);
    topLine->setStyleSheet(QString("background-color: %1; border: none; max-height: 1px; min-height: 1px;").arg(Colors::GRAY_COLOR.name()));
    buttonWrapperLayout->addWidget(topLine);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                     Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    buttonLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 创建确定按钮
    m_confirmBtn = new QPushButton("确定", buttonWidget);
    m_confirmBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setEnabled(false);
    m_confirmBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 确定按钮样式 - 禁用状态背景色为 GRAY_COLOR
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
    
    // 创建取消按钮
    m_cancelBtn = new QPushButton("取消", buttonWidget);
    m_cancelBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 取消按钮样式：背景透明，边框和文字为 GRAY_COLOR
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
    
    // 添加按钮到布局 - 两个按钮各占一半宽度，占满整行
    buttonLayout->addWidget(m_confirmBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    buttonWrapperLayout->addLayout(buttonLayout);
    m_mainLayout->addWidget(buttonWidget);
    
    connect(m_confirmBtn, &QPushButton::clicked, this, &DocumentDialog::onConfirmClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &DocumentDialog::onCancelClicked);
}

QWidget* DocumentDialog::createTabBar()
{
    m_tabBar = new QWidget(this);
    m_tabBar->setStyleSheet("background-color: " + Colors::WHITE_COLOR.name() + ";");

    QHBoxLayout* layout = new QHBoxLayout(m_tabBar);
    layout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                               Dimens::PAGE_PADDING, 0);
    layout->setSpacing(Dimens::PAGE_PADDING);

    m_myDocTab = new QPushButton("我的文档", m_tabBar);
    m_myDocTab->setCursor(Qt::PointingHandCursor);
    m_myDocTab->setFlat(true);

    m_publicDocTab = new QPushButton("公共文档", m_tabBar);
    m_publicDocTab->setCursor(Qt::PointingHandCursor);
    m_publicDocTab->setFlat(true);

    layout->addWidget(m_myDocTab);
    layout->addWidget(m_publicDocTab);
    layout->addStretch();

    connect(m_myDocTab, &QPushButton::clicked, this, &DocumentDialog::onMyDocTabClicked);
    connect(m_publicDocTab, &QPushButton::clicked, this, &DocumentDialog::onPublicDocTabClicked);

    return m_tabBar;
}

void DocumentDialog::updateTabStyles()
{
    const bool myDocActive = (m_pageStack && m_pageStack->currentIndex() == 0);

    // 选中页签：文字与下划线都用主色；
    // 未选中：灰色文字，下划线用白色（与页签底色相同、视觉上不可见），
    // 这样两个页签的边框宽度一致，高度不会差 2px
    auto tabStyle = [](bool active) {
        const QString textColor = active ? Colors::PRIMARY_COLOR.name() : Colors::GRAY_COLOR.name();
        const QString lineColor = active ? Colors::PRIMARY_COLOR.name() : Colors::WHITE_COLOR.name();
        return QString(
            "QPushButton {"
            "   background-color: transparent;"
            "   border: none;"
            "   border-bottom: %1px solid %2;"
            "   color: %3;"
            "   font-size: %4px;"
            "   padding: %5px 0px;"
            "}"
        ).arg(Dimens::STROKE_WIDTH)
         .arg(lineColor)
         .arg(textColor)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Dimens::PAGE_PADDING);
    };

    m_myDocTab->setStyleSheet(tabStyle(myDocActive));
    m_publicDocTab->setStyleSheet(tabStyle(!myDocActive));
}

void DocumentDialog::switchToTab(int index)
{
    if (!m_pageStack || index < 0 || index >= m_pageStack->count()) return;

    m_pageStack->setCurrentIndex(index);
    updateTabStyles();

    // 首次切到公共文档时才去请求
    if (index == 1 && !m_publicLoaded) {
        loadPublicDocList();
    }
}

void DocumentDialog::onMyDocTabClicked()
{
    switchToTab(0);
}

void DocumentDialog::onPublicDocTabClicked()
{
    switchToTab(1);
}

QWidget* DocumentDialog::createMyDocPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // 滚动区域
    m_scrollArea = new QScrollArea(page);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(scrollAreaStyle());
    m_scrollArea->verticalScrollBar()->setStyleSheet(scrollBarStyle());

    m_containerWidget = new QWidget();
    m_containerWidget->setStyleSheet("background-color: " + Colors::WHITE_COLOR.name() + ";");
    m_containerLayout = new QVBoxLayout(m_containerWidget);
    m_containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerLayout->setSpacing(0);
    m_containerLayout->addStretch();

    m_scrollArea->setWidget(m_containerWidget);
    pageLayout->addWidget(m_scrollArea);

    return page;
}

QWidget* DocumentDialog::createPublicDocPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    m_publicScrollArea = new QScrollArea(page);
    m_publicScrollArea->setWidgetResizable(true);
    m_publicScrollArea->setFrameShape(QFrame::NoFrame);
    m_publicScrollArea->setStyleSheet(scrollAreaStyle());
    m_publicScrollArea->verticalScrollBar()->setStyleSheet(scrollBarStyle());

    m_publicContainerWidget = new QWidget();
    m_publicContainerWidget->setStyleSheet("background-color: " + Colors::WHITE_COLOR.name() + ";");
    m_publicContainerLayout = new QVBoxLayout(m_publicContainerWidget);
    m_publicContainerLayout->setContentsMargins(0, 0, 0, 0);
    m_publicContainerLayout->setSpacing(0);
    m_publicContainerLayout->addStretch();

    m_publicScrollArea->setWidget(m_publicContainerWidget);
    pageLayout->addWidget(m_publicScrollArea);

    return page;
}

void DocumentDialog::loadDirectoryList()
{
    QString url = QString("%1?tenantId=%2").arg(Constants::Endpoints::GET_DIRECTORY_LIST).arg(m_tenantId);
    
    // 对话框可能在请求返回前就被关闭（例如点了取消），
    // 用 QPointer 判活，避免回调访问已析构的 this
    QPointer<DocumentDialog> self(this);
    NetworkManager::instance().get(
        url,
        [this, self](const ApiResponse& response) {
            if (!self) return;

            if (response.isSuccess() && !response.data.isNull()) {
                clearDirectoryList();
                m_directoryList.clear();
                
                QJsonArray dirArray = response.data.toJsonArray();
                for (int i = 0; i < dirArray.size(); ++i) {
                    Directory dir = Directory::fromJson(dirArray[i].toObject());
                    if (dir.isValid()) {
                        m_directoryList.append(dir);
                        addDirectoryToUI(dir, i);
                    }
                }
            } else {
                QMessageBox::warning(this, "提示", "加载目录列表失败：" + response.message);
            }
        },
        [this, self](const QString& error) {
            if (!self) return;
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}

void DocumentDialog::addDirectoryToUI(const Directory& dir, int index)
{
    // 目录按钮容器
    QPushButton* dirButton = new QPushButton(m_containerWidget);
    dirButton->setCursor(Qt::PointingHandCursor);
    dirButton->setProperty("directoryIndex", index);
    dirButton->setProperty("directoryId", dir.id);
    dirButton->setFlat(true);
    dirButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    dirButton->setStyleSheet("QPushButton { background-color: transparent; text-align: left; }");
    
    QHBoxLayout* dirLayout = new QHBoxLayout(dirButton);
    dirLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                  Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    dirLayout->setSpacing(Dimens::PAGE_PADDING);
    dirLayout->setAlignment(Qt::AlignVCenter);  // 设置布局垂直居中对齐
    
    // 目录名称
    QLabel* nameLabel = new QLabel(dir.directory, dirButton);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);  // 文字垂直居中对齐
    
    // 箭头按钮
    QPushButton* arrowBtn = new QPushButton(dirButton);
    arrowBtn->setCursor(Qt::PointingHandCursor);
    arrowBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    arrowBtn->setProperty("directoryIndex", index);
    arrowBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    arrowBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    applyArrowIcon(arrowBtn, false);
    
    dirLayout->addWidget(nameLabel, 1);
    dirLayout->addWidget(arrowBtn, 0, Qt::AlignVCenter);  // 箭头按钮垂直居中对齐
    
    // 文档容器（初始隐藏）
    QWidget* docContainer = new QWidget(m_containerWidget);
    docContainer->setVisible(false);
    QVBoxLayout* docLayout = new QVBoxLayout(docContainer);
    docLayout->setContentsMargins(Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    docLayout->setSpacing(0);
    
    // 添加到主布局
    int insertPos = m_containerLayout->count() - 1;
    m_containerLayout->insertWidget(insertPos, dirButton);
    m_containerLayout->insertWidget(insertPos + 1, docContainer);
    
    // 添加分割线 - 颜色为 Colors::GRAY_COLOR
    QFrame* line = new QFrame(m_containerWidget);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet(QString("background-color: %1; border: none; max-height: 1px; min-height: 1px;").arg(Colors::GRAY_COLOR.name()));
    m_containerLayout->insertWidget(insertPos + 2, line);
    
    m_directoryWidgets.append(dirButton);
    m_arrowButtons.append(arrowBtn);
    m_directoryExpanded.append(false);
    m_documentContainers[dir.id] = docContainer;
    
    // 连接点击事件
    connect(dirButton, &QPushButton::clicked, [this, index]() {
        onDirectoryClicked(index);
    });
    connect(arrowBtn, &QPushButton::clicked, [this, index]() {
        onDirectoryClicked(index);
    });
}

void DocumentDialog::onDirectoryClicked(int index)
{
    if (index < 0 || index >= m_directoryList.size()) return;
    
    bool isExpanded = m_directoryExpanded[index];
    const Directory& dir = m_directoryList[index];
    QWidget* docContainer = m_documentContainers[dir.id];
    
    if (isExpanded) {
        // 收起
        docContainer->setVisible(false);
        m_directoryExpanded[index] = false;
        applyArrowIcon(m_arrowButtons[index], false);
    } else {
        // 展开，加载文档列表
        m_currentDirectoryId = dir.id;
        loadDocumentList(dir.id);
    }
}

void DocumentDialog::loadDocumentList(const QString& directoryId)
{
    QString url = QString("%1?tenantId=%2&directoryId=%3")
                      .arg(Constants::Endpoints::GET_DOC_LIST_BY_DIR_ID)
                      .arg(m_tenantId)
                      .arg(directoryId);
    
    // 同 loadDirectoryList：对话框可能已被关闭，回调前先判活
    QPointer<DocumentDialog> self(this);
    NetworkManager::instance().get(
        url,
        [this, self, directoryId](const ApiResponse& response) {
            if (!self) return;

            if (response.isSuccess() && !response.data.isNull()) {
                // 找到对应的索引
                int index = -1;
                for (int i = 0; i < m_directoryList.size(); ++i) {
                    if (m_directoryList[i].id == directoryId) {
                        index = i;
                        break;
                    }
                }
                if (index == -1) return;
                
                // 清空旧的文档列表
                QWidget* docContainer = m_documentContainers[directoryId];
                QLayout* layout = docContainer->layout();
                while (layout->count() > 0) {
                    QLayoutItem* item = layout->takeAt(0);
                    if (item->widget()) {
                        item->widget()->deleteLater();
                    }
                    delete item;
                }
                
                // 清空文档列表数据
                m_documentList.clear();
                m_selectedDocuments.clear();
                m_documentItemWidgets.clear();
                m_currentDirectoryId = directoryId;
                
                // 加载文档列表
                QJsonArray docArray = response.data.toJsonArray();
                for (int i = 0; i < docArray.size(); ++i) {
                    Document doc = Document::fromJson(docArray[i].toObject());
                    if (doc.isValid()) {
                        m_documentList.append(doc);
                        addDocumentToUI(doc, i);
                    }
                }
                
                // 显示容器
                docContainer->setVisible(true);
                m_directoryExpanded[index] = true;
                
                // 箭头变成向下
                applyArrowIcon(m_arrowButtons[index], true);
                
                // 更新确定按钮状态
                updateConfirmButtonState();
            } else {
                QMessageBox::warning(this, "提示", "加载文档列表失败：" + response.message);
            }
        },
        [this, self](const QString& error) {
            if (!self) return;
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}

void DocumentDialog::addDocumentToUI(const Document& doc, int index)
{
    QWidget* docItem = new QWidget();
    docItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    QHBoxLayout* docLayout = new QHBoxLayout(docItem);
    // 文档名称上下间距为 PAGE_PADDING
    docLayout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING);
    docLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 文档名称：name 字段本身已经带了后缀名，不要再拼 ext，否则会出现两个后缀
    QLabel* nameLabel = new QLabel(doc.name, docItem);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // "..." 图标：点击弹出「编辑权限 / 删除」菜单（仅我的文档有）
    QPushButton* moreBtn = new QPushButton(docItem);
    moreBtn->setCursor(Qt::PointingHandCursor);
    moreBtn->setFixedSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE);
    moreBtn->setToolTip("更多");
    moreBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    moreBtn->setIcon(moreDotsIcon(Colors::TEXT_COLOR));
    moreBtn->setIconSize(QSize(Dimens::MIDDLE_ICON_SIZE, Dimens::MIDDLE_ICON_SIZE));
    moreBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // 复选框
    QCheckBox* checkBox = new QCheckBox(docItem);
    checkBox->setCursor(Qt::PointingHandCursor);
    checkBox->setProperty("documentId", doc.id);
    checkBox->setProperty("documentIndex", index);
    checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    connect(checkBox, &QCheckBox::toggled, [this, index](bool checked) {
        onDocumentCheckStateChanged(index, checked);
    });
    connect(moreBtn, &QPushButton::clicked, [this, doc, moreBtn]() {
        showDocumentMenu(doc, moreBtn->mapToGlobal(QPoint(0, moreBtn->height())));
    });
    
    // 顺序：文档名 ... "..."图标 复选框
    docLayout->addWidget(nameLabel, 1);
    docLayout->addWidget(moreBtn, 0, Qt::AlignTop);
    docLayout->addWidget(checkBox, 0, Qt::AlignTop);
    
    QWidget* docContainer = m_documentContainers[m_currentDirectoryId];
    QVBoxLayout* containerLayout = qobject_cast<QVBoxLayout*>(docContainer->layout());
    if (containerLayout) {
        containerLayout->addWidget(docItem);
    }

    m_documentItemWidgets.insert(doc.id, docItem);
}

void DocumentDialog::onDocumentCheckStateChanged(int index, bool checked)
{
    if (checked) {
        m_selectedDocuments[index] = true;
    } else {
        m_selectedDocuments.remove(index);
    }
    updateConfirmButtonState();
}

void DocumentDialog::showDocumentMenu(const Document& doc, const QPoint& globalPos)
{
    QMenu menu(this);
    menu.setStyleSheet(documentMenuStyle());

    QAction* permissionAction = menu.addAction("编辑权限");
    QAction* deleteAction = menu.addAction("删除");

    // 用 triggered 信号而不是 menu.exec() 的返回值：
    // QMenu 在发出 triggered 之前已经先把菜单隐藏了，视觉行为一致，
    // 而且不依赖菜单是被"点击"还是被程序化收起
    connect(permissionAction, &QAction::triggered, this, [this, doc]() {
        openPermissionDialog(doc);
    });
    connect(deleteAction, &QAction::triggered, this, [this, doc]() {
        deleteDocument(doc);
    });

    menu.exec(globalPos);
}

void DocumentDialog::openPermissionDialog(const Document& doc)
{
    DocPermissionDialog dialog(doc, this);
    if (dialog.exec() == QDialog::Accepted) {
        const QString newPermission = dialog.selectedPermission();
        // 同步本地数据，避免再次打开时下拉框还是旧权限
        for (int i = 0; i < m_documentList.size(); ++i) {
            if (m_documentList[i].id == doc.id) {
                m_documentList[i].permission = newPermission;
                break;
            }
        }
        qDebug() << "Doc permission updated:" << doc.id << newPermission;
    }
}

void DocumentDialog::deleteDocument(const Document& doc)
{
    const QMessageBox::StandardButton ret = QMessageBox::question(
        this, "提示", "确定要删除该文档吗？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }

    const QString endpoint = QString(Constants::Endpoints::DELETE_DOC).arg(doc.id);
    qDebug() << "Deleting document:" << endpoint;

    QPointer<DocumentDialog> self(this);
    NetworkManager::instance().del(
        endpoint,
        [this, self, doc](const ApiResponse& response) {
            if (!self) return;
            if (response.isSuccess() && response.data.toInt() > 0) {
                removeDocumentFromUI(doc.id);
            } else {
                QMessageBox::warning(this, "提示",
                    "删除文档失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
            }
        },
        [this, self](const QString& error) {
            if (!self) return;
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}

void DocumentDialog::removeDocumentFromUI(const QString& docId)
{
    // 移除行控件
    QWidget* itemWidget = m_documentItemWidgets.take(docId);
    if (itemWidget) {
        if (QWidget* parent = itemWidget->parentWidget()) {
            if (QLayout* layout = parent->layout()) {
                layout->removeWidget(itemWidget);
            }
        }
        itemWidget->setParent(nullptr);
        itemWidget->deleteLater();
    }

    // 从数据里移除，并把 m_selectedDocuments 的下标整体平移，
    // 否则后面条目的选中状态会全部错位
    int removedIndex = -1;
    for (int i = 0; i < m_documentList.size(); ++i) {
        if (m_documentList[i].id == docId) {
            removedIndex = i;
            break;
        }
    }
    if (removedIndex >= 0) {
        m_documentList.removeAt(removedIndex);

        QMap<int, bool> remapped;
        for (auto it = m_selectedDocuments.constBegin(); it != m_selectedDocuments.constEnd(); ++it) {
            if (it.key() == removedIndex) continue;
            remapped.insert(it.key() > removedIndex ? it.key() - 1 : it.key(), it.value());
        }
        m_selectedDocuments = remapped;
    }

    updateConfirmButtonState();
}

void DocumentDialog::loadPublicDocList()
{
    // 公共文档按当前租户 + 当前公司两个维度过滤
    const QString companyId =
        TokenManager::instance().getValue(Constants::CURRENT_COMPANY_ID_KEY).toString();
    const QString endpoint = Constants::withQuery(Constants::Endpoints::GET_PUBLIC_DOC_LIST,
                                                  {{"tenantId", m_tenantId},
                                                   {"companyId", companyId}});
    qDebug() << "Loading public doc list from:" << endpoint;

    QPointer<DocumentDialog> self(this);
    NetworkManager::instance().get(
        endpoint,
        [this, self](const ApiResponse& response) {
            if (!self) return;

            if (!response.isSuccess() || response.data.isNull()) {
                QMessageBox::warning(this, "提示",
                    "加载公共文档失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
                return;
            }

            m_publicLoaded = true;

            clearPublicDocTree();
            m_publicDocList.clear();

            const QJsonArray docArray = response.data.toJsonArray();
            for (const QJsonValue& value : docArray) {
                const Document doc = Document::fromJson(value.toObject());
                if (doc.isValid()) {
                    m_publicDocList.append(doc);
                }
            }
            qDebug() << "Public doc count:" << m_publicDocList.size();

            buildPublicDocTree();
        },
        [this, self](const QString& error) {
            if (!self) return;
            // 不置 m_publicLoaded，用户再次切到该页签时会重试
            QMessageBox::warning(this, "提示", "加载公共文档失败：" + error);
        }
    );
}

void DocumentDialog::buildPublicDocTree()
{
    m_publicDirectoryNames.clear();
    m_publicDocsByDir.clear();

    // 按 directoryName 分类
    for (const Document& doc : m_publicDocList) {
        QString dirName = doc.directoryName;
        if (dirName.isEmpty()) {
            dirName = doc.directoryId;
        }
        if (!m_publicDocsByDir.contains(dirName)) {
            m_publicDirectoryNames.append(dirName);
        }
        m_publicDocsByDir[dirName].append(doc);
    }

    for (int i = 0; i < m_publicDirectoryNames.size(); ++i) {
        addPublicDirectoryToUI(m_publicDirectoryNames[i], i);
    }

    if (m_publicDirectoryNames.isEmpty()) {
        QLabel* emptyLabel = new QLabel("暂无公共文档", m_publicContainerWidget);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(QString(
            "color: %1;"
            "font-size: %2px;"
            "background-color: transparent;"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::FONT_SIZE_NORMAL));
        m_publicContainerLayout->insertWidget(m_publicContainerLayout->count() - 1, emptyLabel);
    }
}

void DocumentDialog::addPublicDirectoryToUI(const QString& directoryName, int index)
{
    QPushButton* dirButton = new QPushButton(m_publicContainerWidget);
    dirButton->setCursor(Qt::PointingHandCursor);
    dirButton->setProperty("directoryIndex", index);
    dirButton->setProperty("directoryName", directoryName);
    dirButton->setFlat(true);
    dirButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    dirButton->setStyleSheet("QPushButton { background-color: transparent; text-align: left; }");

    QHBoxLayout* dirLayout = new QHBoxLayout(dirButton);
    dirLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                  Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    dirLayout->setSpacing(Dimens::PAGE_PADDING);
    dirLayout->setAlignment(Qt::AlignVCenter);

    QLabel* nameLabel = new QLabel(directoryName, dirButton);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    QPushButton* arrowBtn = new QPushButton(dirButton);
    arrowBtn->setCursor(Qt::PointingHandCursor);
    arrowBtn->setFixedSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE);
    arrowBtn->setProperty("directoryIndex", index);
    arrowBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    arrowBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    applyArrowIcon(arrowBtn, false);

    dirLayout->addWidget(nameLabel, 1);
    dirLayout->addWidget(arrowBtn, 0, Qt::AlignVCenter);

    QWidget* docContainer = new QWidget(m_publicContainerWidget);
    docContainer->setVisible(false);
    QVBoxLayout* docLayout = new QVBoxLayout(docContainer);
    docLayout->setContentsMargins(Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    docLayout->setSpacing(0);

    int insertPos = m_publicContainerLayout->count() - 1;
    m_publicContainerLayout->insertWidget(insertPos, dirButton);
    m_publicContainerLayout->insertWidget(insertPos + 1, docContainer);

    QFrame* line = new QFrame(m_publicContainerWidget);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet(QString("background-color: %1; border: none; max-height: 1px; min-height: 1px;").arg(Colors::GRAY_COLOR.name()));
    m_publicContainerLayout->insertWidget(insertPos + 2, line);

    m_publicDirectoryWidgets.append(dirButton);
    m_publicArrowButtons.append(arrowBtn);
    m_publicDirectoryExpanded.append(false);
    m_publicDocContainers[directoryName] = docContainer;

    // 公共文档一次性全部拿到，这里直接建好条目，展开时无需再请求
    const QList<Document> docs = m_publicDocsByDir.value(directoryName);
    for (const Document& doc : docs) {
        addPublicDocumentToUI(doc, directoryName);
    }

    connect(dirButton, &QPushButton::clicked, [this, index]() {
        onPublicDirectoryClicked(index);
    });
    connect(arrowBtn, &QPushButton::clicked, [this, index]() {
        onPublicDirectoryClicked(index);
    });
}

void DocumentDialog::onPublicDirectoryClicked(int index)
{
    if (index < 0 || index >= m_publicDirectoryNames.size()) return;

    const QString dirName = m_publicDirectoryNames[index];
    QWidget* docContainer = m_publicDocContainers.value(dirName);
    if (!docContainer) return;

    const bool isExpanded = m_publicDirectoryExpanded[index];
    m_publicDirectoryExpanded[index] = !isExpanded;
    docContainer->setVisible(!isExpanded);
    applyArrowIcon(m_publicArrowButtons[index], !isExpanded);
}

void DocumentDialog::addPublicDocumentToUI(const Document& doc, const QString& directoryName)
{
    QWidget* docContainer = m_publicDocContainers.value(directoryName);
    if (!docContainer) return;

    QWidget* docItem = new QWidget();
    docItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout* docLayout = new QHBoxLayout(docItem);
    docLayout->setContentsMargins(0, Dimens::PAGE_PADDING, 0, Dimens::PAGE_PADDING);
    docLayout->setSpacing(Dimens::PAGE_PADDING);

    // 文档名称：name 已包含后缀名，不再拼 ext
    QLabel* nameLabel = new QLabel(doc.name, docItem);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 公共文档不是自己的文档，不提供编辑权限/删除，因此没有 "..." 图标
    QCheckBox* checkBox = new QCheckBox(docItem);
    checkBox->setCursor(Qt::PointingHandCursor);
    checkBox->setProperty("documentId", doc.id);
    checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(checkBox, &QCheckBox::toggled, [this, doc](bool checked) {
        onPublicDocumentCheckStateChanged(doc.id, checked);
    });

    docLayout->addWidget(nameLabel, 1);
    docLayout->addWidget(checkBox, 0, Qt::AlignTop);

    QVBoxLayout* containerLayout = qobject_cast<QVBoxLayout*>(docContainer->layout());
    if (containerLayout) {
        containerLayout->addWidget(docItem);
    }
}

void DocumentDialog::onPublicDocumentCheckStateChanged(const QString& docId, bool checked)
{
    if (checked) {
        m_publicSelectedDocuments[docId] = true;
    } else {
        m_publicSelectedDocuments.remove(docId);
    }
    updateConfirmButtonState();
}

void DocumentDialog::clearPublicDocTree()
{
    while (m_publicContainerLayout->count() > 1) {
        QLayoutItem* item = m_publicContainerLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    m_publicDirectoryWidgets.clear();
    m_publicArrowButtons.clear();
    m_publicDirectoryExpanded.clear();
    m_publicDocContainers.clear();
    m_publicDirectoryNames.clear();
    m_publicDocsByDir.clear();
    m_publicSelectedDocuments.clear();
}

void DocumentDialog::updateConfirmButtonState()
{
    bool hasSelection = !m_selectedDocuments.isEmpty() || !m_publicSelectedDocuments.isEmpty();
    
    if (hasSelection) {
        m_confirmBtn->setEnabled(true);
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
        m_confirmBtn->setEnabled(false);
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

QStringList DocumentDialog::getSelectedDocumentIds() const
{
    QStringList ids;

    // 我的文档：按索引取（原有逻辑）
    for (auto it = m_selectedDocuments.begin(); it != m_selectedDocuments.end(); ++it) {
        int index = it.key();
        if (index >= 0 && index < m_documentList.size()) {
            ids.append(m_documentList[index].id);
        }
    }

    // 公共文档：按文档 id 取
    for (auto it = m_publicSelectedDocuments.begin(); it != m_publicSelectedDocuments.end(); ++it) {
        if (!ids.contains(it.key())) {
            ids.append(it.key());
        }
    }

    return ids;
}

void DocumentDialog::onConfirmClicked()
{
    accept();
}

void DocumentDialog::onCancelClicked()
{
    reject();
}

void DocumentDialog::clearDirectoryList()
{
    while (m_containerLayout->count() > 1) {
        QLayoutItem* item = m_containerLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    m_directoryWidgets.clear();
    m_arrowButtons.clear();
    m_directoryExpanded.clear();
    m_documentContainers.clear();
    m_documentItemWidgets.clear();
}

void DocumentDialog::clearDocumentList()
{
    m_documentList.clear();
    m_selectedDocuments.clear();
}
