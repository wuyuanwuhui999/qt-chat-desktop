#include "DocPermissionDialog.h"
#include "network/NetworkManager.h"
#include "config/Constants.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include <QPointer>
#include <QMessageBox>
#include <QJsonObject>
#include <QDebug>

QStringList DocPermissionDialog::permissionValues()
{
    return QStringList() << "private" << "tenant" << "company";
}

QString DocPermissionDialog::permissionLabel(const QString& value)
{
    if (value == "private") return "私密";
    if (value == "tenant")  return "租户内公开";
    if (value == "company") return "公司内公开";
    return value;
}

DocPermissionDialog::DocPermissionDialog(const Document& doc, QWidget* parent)
    : QDialog(parent)
    , m_doc(doc)
{
    // 标题由窗口自身标题栏显示，内容区不再重复画标题
    setWindowTitle("编辑文档权限");
    setFixedSize(420, 260);
    setModal(true);

    setupUI();
}

void DocPermissionDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                     Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    m_mainLayout->setSpacing(Dimens::PAGE_PADDING);

    // 内容区（灰底，占满剩余空间）
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("docPermissionContent");
    m_contentWidget->setStyleSheet(QString(
        "QWidget#docPermissionContent {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::BACKGROUND_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    QVBoxLayout* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                      Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    contentLayout->setSpacing(Dimens::PAGE_PADDING);

    // 权限下拉框：单行控件，高度 INPUT_HEIGHT、圆角 INPUT_HEIGHT/2
    m_permissionCombo = new QComboBox(m_contentWidget);
    m_permissionCombo->setFixedHeight(Dimens::INPUT_HEIGHT);
    m_permissionCombo->setCursor(Qt::PointingHandCursor);
    m_permissionCombo->setStyleSheet(QString(
        "QComboBox {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: %4px;"
        "   padding: 0 %5px;"
        "   font-size: %6px;"
        "}"
        "QComboBox:focus {"
        "   border-color: %7;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: %8px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   selection-background-color: %7;"
        "   selection-color: %9;"
        "   outline: none;"
        "}"
    ).arg(Colors::WHITE_COLOR.name())
     .arg(Colors::TEXT_COLOR.name())
     .arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::INPUT_HEIGHT / 2)
     .arg(Dimens::PAGE_PADDING)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.name())
     .arg(Dimens::INPUT_HEIGHT / 2)
     .arg(Colors::WHITE_COLOR.name()));

    // 下拉选项：private / tenant / company
    const QStringList values = permissionValues();
    for (const QString& value : values) {
        m_permissionCombo->addItem(permissionLabel(value), value);
    }

    // 用文档自身的 permission 字段定位当前选中项
    const QString current = m_doc.permission;
    int currentIndex = m_permissionCombo->findData(current);
    if (currentIndex < 0) {
        // 后端没有返回可识别的权限值时，退回第一项
        currentIndex = 0;
        qDebug() << "Unknown document permission, fallback to first option. permission =" << current;
    }
    m_permissionCombo->setCurrentIndex(currentIndex);
    m_selectedPermission = m_permissionCombo->currentData().toString();

    contentLayout->addWidget(m_permissionCombo);
    contentLayout->addStretch();

    // 确定 / 取消
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(Dimens::PAGE_PADDING);

    m_confirmBtn = new QPushButton("确定", m_contentWidget);
    m_confirmBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 确定按钮：主色背景 + 白字
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
    contentLayout->addLayout(buttonLayout);

    m_mainLayout->addWidget(m_contentWidget, 1);

    connect(m_confirmBtn, &QPushButton::clicked, this, &DocPermissionDialog::onConfirmClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void DocPermissionDialog::onConfirmClicked()
{
    m_selectedPermission = m_permissionCombo->currentData().toString();

    QJsonObject data;
    data["docId"] = m_doc.id;
    data["permission"] = m_selectedPermission;

    qDebug() << "Updating doc permission:" << m_doc.id << m_selectedPermission;

    QPointer<DocPermissionDialog> self(this);
    NetworkManager::instance().put(
        Constants::Endpoints::UPDATE_DOC_PERMISSION,
        data,
        [this, self](const ApiResponse& response) {
            if (!self) return;
            if (response.isSuccess() && response.data.toInt() > 0) {
                accept();
            } else {
                QMessageBox::warning(this, "提示",
                    "更新文档权限失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
            }
        },
        [this, self](const QString& error) {
            if (!self) return;
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}
