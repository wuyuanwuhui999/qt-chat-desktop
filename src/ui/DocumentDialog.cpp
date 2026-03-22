// DocumentDialog.cpp
#include "DocumentDialog.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPropertyAnimation>

DocumentDialog::DocumentDialog(const QString& tenantId, QWidget *parent)
    : QDialog(parent)
    , m_tenantId(tenantId)
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
    
    // 滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");
    m_scrollArea->verticalScrollBar()->setStyleSheet(
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
    
    m_containerWidget = new QWidget();
    m_containerWidget->setStyleSheet("background-color: white;");
    m_containerLayout = new QVBoxLayout(m_containerWidget);
    m_containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerLayout->setSpacing(0);
    m_containerLayout->addStretch();
    
    m_scrollArea->setWidget(m_containerWidget);
    m_mainLayout->addWidget(m_scrollArea);
    
    // 底部按钮容器
    QWidget* buttonWidget = new QWidget(this);
    buttonWidget->setStyleSheet("background-color: white;");
    
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

void DocumentDialog::loadDirectoryList()
{
    QString url = QString("%1?tenantId=%2").arg(Constants::Endpoints::GET_DIRECTORY_LIST).arg(m_tenantId);
    
    NetworkManager::instance().get(
        url,
        [this](const ApiResponse& response) {
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
        [this](const QString& error) {
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
    
    QPixmap arrowPixmap(":/images/icon_down.png");
    if (!arrowPixmap.isNull()) {
        // 设置透明度 0.5
        QPixmap transparentPixmap(arrowPixmap.size());
        transparentPixmap.fill(Qt::transparent);
        QPainter painter(&transparentPixmap);
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, arrowPixmap);
        
        QPixmap rotatedPixmap(transparentPixmap.size());
        rotatedPixmap.fill(Qt::transparent);
        QPainter rotatePainter(&rotatedPixmap);
        rotatePainter.translate(rotatedPixmap.width() / 2, rotatedPixmap.height() / 2);
        rotatePainter.rotate(-90);
        rotatePainter.translate(-rotatedPixmap.width() / 2, -rotatedPixmap.height() / 2);
        rotatePainter.drawPixmap(0, 0, transparentPixmap);
        arrowBtn->setIcon(QIcon(rotatedPixmap));
        arrowBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
    }
    arrowBtn->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    
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
        
        // 箭头变回向右
        QPixmap arrowPixmap(":/images/icon_down.png");
        if (!arrowPixmap.isNull()) {
            QPixmap transparentPixmap(arrowPixmap.size());
            transparentPixmap.fill(Qt::transparent);
            QPainter painter(&transparentPixmap);
            painter.setOpacity(0.5);
            painter.drawPixmap(0, 0, arrowPixmap);
            
            QPixmap rotatedPixmap(transparentPixmap.size());
            rotatedPixmap.fill(Qt::transparent);
            QPainter rotatePainter(&rotatedPixmap);
            rotatePainter.translate(rotatedPixmap.width() / 2, rotatedPixmap.height() / 2);
            rotatePainter.rotate(-90);
            rotatePainter.translate(-rotatedPixmap.width() / 2, -rotatedPixmap.height() / 2);
            rotatePainter.drawPixmap(0, 0, transparentPixmap);
            m_arrowButtons[index]->setIcon(QIcon(rotatedPixmap));
            m_arrowButtons[index]->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
        }
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
    
    NetworkManager::instance().get(
        url,
        [this, directoryId](const ApiResponse& response) {
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
                QPixmap arrowPixmap(":/images/icon_down.png");
                if (!arrowPixmap.isNull()) {
                    QPixmap transparentPixmap(arrowPixmap.size());
                    transparentPixmap.fill(Qt::transparent);
                    QPainter painter(&transparentPixmap);
                    painter.setOpacity(0.5);
                    painter.drawPixmap(0, 0, arrowPixmap);
                    m_arrowButtons[index]->setIcon(QIcon(transparentPixmap));
                    m_arrowButtons[index]->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
                }
                
                // 更新确定按钮状态
                updateConfirmButtonState();
            } else {
                QMessageBox::warning(this, "提示", "加载文档列表失败：" + response.message);
            }
        },
        [this](const QString& error) {
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
    
    // 文档名称 - 高度自适应
    QString displayName = doc.name;
    if (!doc.ext.isEmpty()) {
        displayName += "." + doc.ext;
    }
    QLabel* nameLabel = new QLabel(displayName, docItem);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    // 复选框
    QCheckBox* checkBox = new QCheckBox(docItem);
    checkBox->setCursor(Qt::PointingHandCursor);
    checkBox->setProperty("documentId", doc.id);
    checkBox->setProperty("documentIndex", index);
    checkBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    connect(checkBox, &QCheckBox::toggled, [this, index](bool checked) {
        onDocumentCheckStateChanged(index, checked);
    });
    
    docLayout->addWidget(nameLabel, 1);
    docLayout->addWidget(checkBox, 0, Qt::AlignTop);
    
    QWidget* docContainer = m_documentContainers[m_currentDirectoryId];
    QVBoxLayout* containerLayout = qobject_cast<QVBoxLayout*>(docContainer->layout());
    if (containerLayout) {
        containerLayout->addWidget(docItem);
    }
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

void DocumentDialog::updateConfirmButtonState()
{
    bool hasSelection = !m_selectedDocuments.isEmpty();
    
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
    for (auto it = m_selectedDocuments.begin(); it != m_selectedDocuments.end(); ++it) {
        int index = it.key();
        if (index >= 0 && index < m_documentList.size()) {
            ids.append(m_documentList[index].id);
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
}

void DocumentDialog::clearDocumentList()
{
    m_documentList.clear();
    m_selectedDocuments.clear();
}