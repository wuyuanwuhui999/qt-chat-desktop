#include "CompanyWindow.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include <QHBoxLayout>
#include <QRadioButton>
#include <QFrame>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>
#include <QMessageBox>
#include <QDebug>

CompanyWindow::CompanyWindow(QWidget *parent)
    : QWidget(parent)
    , radioGroup(new QButtonGroup(this))
{
    // 背景色与登录页保持一致
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QBrush(Colors::BACKGROUND_COLOR));
    setPalette(palette);

    setupUI();
    loadCompanyList();
}

void CompanyWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    mainLayout->setSpacing(Dimens::PAGE_PADDING);

    // 白色圆角卡片
    card = new QWidget(this);
    card->setObjectName("companyCard");
    card->setFixedWidth(500);
    card->setStyleSheet(QString(
        "QWidget#companyCard {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::WHITE_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    cardLayout->setSpacing(Dimens::PAGE_PADDING);

    // 标题
    titleLabel = new QLabel("选择公司", card);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "font-weight: bold;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_BIG));

    // 公司列表（超出高度可滚动）
    scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setFixedHeight(Dimens::LIST_HEIGHT);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    scrollArea->verticalScrollBar()->setStyleSheet(
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

    listContainer = new QWidget();
    listContainer->setStyleSheet("background-color: transparent;");
    listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(Dimens::PAGE_PADDING);
    listLayout->addStretch();  // 弹簧，使条目靠上
    scrollArea->setWidget(listContainer);

    // 确定按钮
    confirmBtn = new QPushButton("确定", card);
    confirmBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(confirmBtn, &QPushButton::clicked, this, &CompanyWindow::onConfirmClicked);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(scrollArea);
    cardLayout->addWidget(confirmBtn);

    mainLayout->addStretch(1);
    mainLayout->addWidget(card, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);

    updateConfirmButtonState();
}

void CompanyWindow::loadCompanyList() {
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        NetworkManager::instance().setAuthToken(token);
    }

    qDebug() << "Loading company list from:" << Constants::Endpoints::GET_COMPANY_LIST;

    NetworkManager::instance().get(
        Constants::Endpoints::GET_COMPANY_LIST,
        [this](const ApiResponse& response) {
            qDebug() << "Company list response - status:" << response.status
                     << "message:" << response.message;

            if (!response.isSuccess() || response.data.isNull()) {
                QMessageBox::warning(this, "提示",
                    "加载公司列表失败：" + (response.message.isEmpty() ? "未知错误" : response.message));
                return;
            }

            clearCompanyList();

            const QJsonArray companyArray = response.data.toJsonArray();
            qDebug() << "Company array size:" << companyArray.size();

            for (const QJsonValue& value : companyArray) {
                const Company company = Company::fromJson(value.toObject());
                if (company.isValid()) {
                    companyList.append(company);
                    addCompanyItem(company);
                }
            }

            if (companyList.isEmpty()) {
                QLabel* emptyLabel = new QLabel("暂无可选公司", listContainer);
                emptyLabel->setAlignment(Qt::AlignCenter);
                emptyLabel->setStyleSheet(QString(
                    "color: %1;"
                    "font-size: %2px;"
                    "background-color: transparent;"
                ).arg(Colors::GRAY_COLOR.name())
                 .arg(Dimens::FONT_SIZE_NORMAL));
                listLayout->insertWidget(listLayout->count() - 1, emptyLabel);
                return;
            }

            // 用缓存里的公司 id 自动选中该公司
            const QString cachedCompanyId =
                TokenManager::instance().getValue(Constants::CURRENT_COMPANY_ID_KEY).toString();
            qDebug() << "Cached company id:" << cachedCompanyId;

            if (!cachedCompanyId.isEmpty()) {
                selectCompanyById(cachedCompanyId);
            }
        },
        [this](const QString& error) {
            qDebug() << "Network error when loading company list:" << error;
            QMessageBox::warning(this, "提示", "加载公司列表失败：" + error);
        }
    );
}

void CompanyWindow::addCompanyItem(const Company& company) {
    QWidget* item = new QWidget(listContainer);
    item->setStyleSheet(QString(
        "QWidget {"
        "   background-color: %1;"
        "   border-radius: %2px;"
        "}"
    ).arg(Colors::SEARCH_INPUT_COLOR.name())
     .arg(Dimens::MODULE_BORDER_RADIUS));

    QHBoxLayout* itemLayout = new QHBoxLayout(item);
    itemLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    itemLayout->setSpacing(Dimens::PAGE_PADDING);

    // 公司名称 + 编码/描述
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(Dimens::PAGE_PADDING);

    QLabel* nameLabel = new QLabel(company.name, item);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "font-weight: bold;"
        "background-color: transparent;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));

    QString subText = company.description;
    if (subText.isEmpty()) {
        subText = company.code;
    }
    QLabel* subLabel = new QLabel(subText, item);
    subLabel->setWordWrap(true);
    subLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
        "background-color: transparent;"
    ).arg(Colors::SUB_TITLE_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL - 2));

    textLayout->addWidget(nameLabel);
    if (!subText.isEmpty()) {
        textLayout->addWidget(subLabel);
    }

    QRadioButton* radioBtn = new QRadioButton(item);
    radioBtn->setCursor(Qt::PointingHandCursor);
    radioBtn->setProperty("companyId", company.id);

    connect(radioBtn, &QRadioButton::toggled, this, [this, company](bool checked) {
        if (!checked) return;
        selectedCompany = company;
        updateConfirmButtonState();
    });

    radioGroup->addButton(radioBtn);

    itemLayout->addLayout(textLayout, 1);
    itemLayout->addWidget(radioBtn, 0, Qt::AlignVCenter);

    listLayout->insertWidget(listLayout->count() - 1, item);
}

void CompanyWindow::clearCompanyList() {
    // 清空数据与选中状态（保留末尾的弹簧）
    while (listLayout->count() > 1) {
        QLayoutItem* item = listLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const QList<QAbstractButton*> buttons = radioGroup->buttons();
    for (QAbstractButton* btn : buttons) {
        radioGroup->removeButton(btn);
    }

    companyList.clear();
    selectedCompany = Company();
    updateConfirmButtonState();
}

void CompanyWindow::selectCompanyById(const QString& companyId) {
    const QList<QAbstractButton*> buttons = radioGroup->buttons();
    for (QAbstractButton* btn : buttons) {
        if (btn->property("companyId").toString() == companyId) {
            // setChecked 会触发 toggled，从而写入 selectedCompany
            btn->setChecked(true);
            qDebug() << "Auto-selected cached company:" << companyId;
            return;
        }
    }
    qDebug() << "Cached company not found in list:" << companyId;
}

void CompanyWindow::updateConfirmButtonState() {
    const bool enabled = selectedCompany.isValid();
    confirmBtn->setEnabled(enabled);

    if (enabled) {
        confirmBtn->setStyleSheet(QString(
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
        confirmBtn->setStyleSheet(QString(
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

void CompanyWindow::onConfirmClicked() {
    if (!selectedCompany.isValid()) {
        QMessageBox::warning(this, "提示", "请选择公司");
        return;
    }

    // 缓存选中的公司条目与公司 id
    TokenManager::instance().setValue(
        Constants::CURRENT_COMPANY_KEY,
        QString::fromUtf8(QJsonDocument(selectedCompany.toJson()).toJson(QJsonDocument::Compact)));
    TokenManager::instance().setValue(Constants::CURRENT_COMPANY_ID_KEY, selectedCompany.id);

    qDebug() << "Company selected:" << selectedCompany.id << selectedCompany.name;

    emit companySelected(selectedCompany.id);
}
