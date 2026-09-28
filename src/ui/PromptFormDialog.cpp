#include "PromptFormDialog.h"
#include "network/NetworkManager.h"
#include "config/Constants.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include <QFrame>
#include <QPointer>
#include <QMessageBox>
#include <QJsonObject>
#include <QDebug>

PromptFormDialog::PromptFormDialog(Mode mode,
                                   const QString& tenantId,
                                   const Prompt& prompt,
                                   QWidget* parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_tenantId(tenantId)
    , m_prompt(prompt)
{
    setWindowTitle(m_mode == Add ? "添加提示词" : "更新提示词");
    setFixedSize(560, 480);
    setModal(true);

    setupUI();
}

void PromptFormDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                     Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    m_mainLayout->setSpacing(Dimens::PAGE_PADDING);

    // 标题由窗口自身标题栏显示（setWindowTitle），内容区不再重复画一遍标题

    // 内容区（灰底，占满剩余空间）
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("promptFormContent");
    m_contentWidget->setStyleSheet(QString(
        "QWidget#promptFormContent {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::BACKGROUND_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                        Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    m_contentLayout->setSpacing(Dimens::PAGE_PADDING);

    // 文本框卡片：占满内容区剩余高度
    m_card = new QWidget(m_contentWidget);
    m_card->setObjectName("promptFormCard");
    m_card->setStyleSheet(QString(
        "QWidget#promptFormCard {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    QVBoxLayout* cardLayout = new QVBoxLayout(m_card);
    cardLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    cardLayout->setSpacing(Dimens::PAGE_PADDING);

    m_promptEdit = new QTextEdit(m_card);
    m_promptEdit->setPlaceholderText("请输入提示词");
    m_promptEdit->setFrameStyle(QFrame::NoFrame);
    m_promptEdit->setMinimumHeight(Dimens::INPUT_HEIGHT);
    m_promptEdit->setStyleSheet(QString(
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
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::GRAY_COLOR.name()));
    cardLayout->addWidget(m_promptEdit);

    // 编辑模式：显示原有提示词文本
    if (m_mode == Update && !m_prompt.prompt.isEmpty()) {
        m_promptEdit->setPlainText(m_prompt.prompt);
    }

    m_contentLayout->addWidget(m_card, 1);

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

    m_mainLayout->addWidget(m_contentWidget, 1);

    connect(m_confirmBtn, &QPushButton::clicked, this, &PromptFormDialog::onConfirmClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_promptEdit, &QTextEdit::textChanged, this, &PromptFormDialog::onTextChanged);

    onTextChanged();
}

void PromptFormDialog::onTextChanged()
{
    updateConfirmButtonState();
}

void PromptFormDialog::updateConfirmButtonState()
{
    const bool hasText = !m_promptEdit->toPlainText().trimmed().isEmpty();
    m_confirmBtn->setEnabled(hasText);

    if (hasText) {
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
        // 禁用状态：背景 GRAY_COLOR
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

void PromptFormDialog::onConfirmClicked()
{
    submit();
}

void PromptFormDialog::submit()
{
    const QString text = m_promptEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    QPointer<PromptFormDialog> self(this);

    if (m_mode == Add) {
        QJsonObject data;
        data["tenantId"] = m_tenantId;
        data["prompt"] = text;

        NetworkManager::instance().post(
            Constants::Endpoints::INSERT_PROMPT,
            data,
            [this, self, text](const ApiResponse& response) {
                if (!self) return;
                if (response.isSuccess() && response.data.toInt() > 0) {
                    m_prompt.prompt = text;
                    accept();
                } else {
                    QMessageBox::warning(this, "提示",
                        "添加提示词失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
                }
            },
            [this, self](const QString& error) {
                if (!self) return;
                QMessageBox::warning(this, "提示", "网络错误：" + error);
            }
        );
    } else {
        QJsonObject data;
        data["id"] = m_prompt.id;
        data["prompt"] = text;

        NetworkManager::instance().put(
            Constants::Endpoints::UPDATE_PROMPT,
            data,
            [this, self, text](const ApiResponse& response) {
                if (!self) return;
                if (response.isSuccess() && response.data.toInt() > 0) {
                    m_prompt.prompt = text;
                    accept();
                } else {
                    QMessageBox::warning(this, "提示",
                        "更新提示词失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
                }
            },
            [this, self](const QString& error) {
                if (!self) return;
                QMessageBox::warning(this, "提示", "网络错误：" + error);
            }
        );
    }
}
