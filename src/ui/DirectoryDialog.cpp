#include "DirectoryDialog.h"
#include "network/NetworkManager.h"
#include "utils/TokenManager.h"
#include "config/Constants.h"
#include <QJsonArray>
#include <QListWidgetItem>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QDebug>

DirectoryDialog::DirectoryDialog(const QString& tenantId, QWidget *parent)
    : QDialog(parent)
    , m_tenantId(tenantId)
    , radioGroup(new QButtonGroup(this))
{
    setWindowTitle("上传文档");
    setFixedSize(500, 400);
    setModal(true);
    
    setupUI();
    loadDirectoryList();
}

DirectoryDialog::~DirectoryDialog()
{
}

void DirectoryDialog::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Dimens::PAGE_PADDING, 0,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    mainLayout->setSpacing(Dimens::PAGE_PADDING);

    // 目录列表
    directoryListWidget = new QListWidget(this);
    directoryListWidget->setFrameShape(QFrame::NoFrame);
    directoryListWidget->setStyleSheet(
        "QListWidget {"
        "   outline: none;"
        "   background-color: transparent;"
        "}"
        "QListWidget::item {"
        "   border-bottom: 1px solid " + Colors::GRAY_COLOR.name() + ";"
        "   padding: 0px;"
        "}"
        "QListWidget::item:last {"
        "   border-bottom: none;"
        "}"
    );
    
    mainLayout->addWidget(directoryListWidget);

    // 创建目录按钮
    createDirBtn = new QPushButton("创建目录", this);
    createDirBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    createDirBtn->setCursor(Qt::PointingHandCursor);
    createDirBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: white;"
        "   border: none;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %4;"
        "}"
    ).arg(Colors::PRIMARY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.lighter(110).name()));
    
    connect(createDirBtn, &QPushButton::clicked, this, &DirectoryDialog::onCreateDirClicked);
    mainLayout->addWidget(createDirBtn);

    // 创建目录输入框（初始隐藏）
    createInputWidget = new QWidget(this);
    createInputWidget->setVisible(false);
    createInputLayout = new QHBoxLayout(createInputWidget);
    createInputLayout->setContentsMargins(0, 0, 0, 0);
    createInputLayout->setSpacing(Dimens::PAGE_PADDING);

    dirNameEdit = new QLineEdit(createInputWidget);
    dirNameEdit->setPlaceholderText("请输入目录名称");
    dirNameEdit->setFixedHeight(Dimens::INPUT_HEIGHT);
    dirNameEdit->setStyleSheet(QString(
        "QLineEdit {"
        "   border: 1px solid %1;"
        "   border-radius: %2px;"
        "   padding: 0 %3px;"
        "   font-size: %4px;"
        "}"
        "QLineEdit:focus {"
        "   border-color: %5;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::INPUT_HEIGHT / 2)
     .arg(Dimens::PAGE_PADDING)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.name()));

    // 确认按钮
    confirmCreateBtn = new QPushButton(createInputWidget);
    confirmCreateBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    confirmCreateBtn->setCursor(Qt::PointingHandCursor);
    
    // 关闭按钮
    closeCreateBtn = new QPushButton(createInputWidget);
    closeCreateBtn->setFixedSize(Dimens::BTN_HEIGHT, Dimens::BTN_HEIGHT);
    closeCreateBtn->setCursor(Qt::PointingHandCursor);
    
    updateCreateInputButtonsStyle();

    createInputLayout->addWidget(dirNameEdit, 1);
    createInputLayout->addWidget(confirmCreateBtn);
    createInputLayout->addWidget(closeCreateBtn);

    connect(confirmCreateBtn, &QPushButton::clicked, this, &DirectoryDialog::onConfirmCreateDir);
    connect(closeCreateBtn, &QPushButton::clicked, this, &DirectoryDialog::onCloseCreateInput);

    mainLayout->addWidget(createInputWidget);

    // 确定和取消按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(Dimens::PAGE_PADDING);

    confirmBtn = new QPushButton("确定", this);
    confirmBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setEnabled(false);
    confirmBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: white;"
        "   border: none;"
        "   border-radius: %2px;"
        "   font-size: %3px;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL));

    cancelBtn = new QPushButton("取消", this);
    cancelBtn->setFixedHeight(Dimens::BTN_HEIGHT);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: white;"
        "   color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: %3px;"
        "   font-size: %4px;"
        "}"
        "QPushButton:hover {"
        "   border-color: %5;"
        "   color: %5;"
        "}"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Dimens::FONT_SIZE_NORMAL)
     .arg(Colors::PRIMARY_COLOR.name()));

    buttonLayout->addWidget(confirmBtn);
    buttonLayout->addWidget(cancelBtn);
    
    mainLayout->addSpacing(Dimens::PAGE_PADDING);
    mainLayout->addLayout(buttonLayout);

    connect(confirmBtn, &QPushButton::clicked, this, &DirectoryDialog::onConfirmUpload);
    connect(cancelBtn, &QPushButton::clicked, this, &DirectoryDialog::reject);
}

void DirectoryDialog::updateCreateInputButtonsStyle()
{
    // 确认按钮样式
    QPixmap surePixmap(":/images/icon_sure.png");
    if (!surePixmap.isNull()) {
        QPixmap scaledPixmap = surePixmap.scaled(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE,
                                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
        confirmCreateBtn->setIcon(QIcon(scaledPixmap));
        confirmCreateBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
    }
    
    confirmCreateBtn->setStyleSheet(QString(
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

    // 关闭按钮样式
    QPixmap closePixmap(":/images/icon_close.png");
    if (!closePixmap.isNull()) {
        QPixmap scaledPixmap = closePixmap.scaled(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE,
                                                  Qt::KeepAspectRatio, Qt::SmoothTransformation);
        closeCreateBtn->setIcon(QIcon(scaledPixmap));
        closeCreateBtn->setIconSize(QSize(Dimens::SMALL_ICON_SIZE, Dimens::SMALL_ICON_SIZE));
    }
    
    closeCreateBtn->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1;"
        "   border: none;"
        "   border-radius: %2px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %3;"
        "}"
    ).arg(Colors::GRAY_COLOR.name())
     .arg(Dimens::BTN_HEIGHT / 2)
     .arg(Colors::GRAY_COLOR.darker(110).name()));
}

void DirectoryDialog::loadDirectoryList()
{
    QString url = QString("%1?tenantId=%2").arg(Constants::Endpoints::GET_DIRECTORY_LIST).arg(m_tenantId);
    
    NetworkManager::instance().get(
        url,
        [this](const ApiResponse& response) {
            if (response.isSuccess() && !response.data.isNull()) {
                clearDirectoryList();
                directoryList.clear();
                
                QJsonArray dirArray = response.data.toJsonArray();
                for (const QJsonValue& value : dirArray) {
                    Directory dir = Directory::fromJson(value.toObject());
                    if (dir.isValid()) {
                        directoryList.append(dir);
                        addDirectoryToList(dir);
                    }
                }
            } else {
                QMessageBox::warning(this, "提示", "加载目录列表失败：" + response.message);
            }
        },
        [this](const QString& error) {
            QMessageBox::warning(this, "提示", "加载目录列表失败：" + error);
        }
    );
}

void DirectoryDialog::addDirectoryToList(const Directory& dir)
{
    QListWidgetItem* item = new QListWidgetItem(directoryListWidget);
    
    QWidget* itemWidget = new QWidget();
    QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
    itemLayout->setContentsMargins(Dimens::PAGE_PADDING, Dimens::PAGE_PADDING,
                                   Dimens::PAGE_PADDING, Dimens::PAGE_PADDING);
    itemLayout->setSpacing(Dimens::PAGE_PADDING);
    
    // 目录名称
    QLabel* nameLabel = new QLabel(dir.directory);
    nameLabel->setStyleSheet(QString(
        "color: %1;"
        "font-size: %2px;"
    ).arg(Colors::TEXT_COLOR.name())
     .arg(Dimens::FONT_SIZE_NORMAL));
    
    // 单选按钮
    QRadioButton* radioBtn = new QRadioButton();
    radioBtn->setProperty("directoryId", dir.id);
    radioBtn->setProperty("directoryName", dir.directory);
    
    connect(radioBtn, &QRadioButton::toggled, [this, radioBtn](bool checked) {
        if (checked) {
            m_selectedDirectoryId = radioBtn->property("directoryId").toString();
            m_selectedDirectoryName = radioBtn->property("directoryName").toString();
            updateConfirmButtonState();
        }
    });
    
    radioGroup->addButton(radioBtn);
    
    itemLayout->addWidget(nameLabel, 1);
    itemLayout->addWidget(radioBtn);
    
    item->setSizeHint(itemWidget->sizeHint());
    directoryListWidget->setItemWidget(item, itemWidget);
}

void DirectoryDialog::clearDirectoryList()
{
    directoryListWidget->clear();
    directoryList.clear();
    
    // 清空单选按钮组
    QList<QAbstractButton*> buttons = radioGroup->buttons();
    for (QAbstractButton* btn : buttons) {
        radioGroup->removeButton(btn);
    }
}

void DirectoryDialog::onCreateDirClicked()
{
    showCreateInput();
}

void DirectoryDialog::onConfirmCreateDir()
{
    QString dirName = dirNameEdit->text().trimmed();
    if (dirName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入目录名称");
        return;
    }
    
    // 调用创建目录接口
    QJsonObject data;
    data["directory"] = dirName;
    data["tenantId"] = m_tenantId;
    
    NetworkManager::instance().post(
        Constants::Endpoints::CREATE_DIR,
        data,
        [this](const ApiResponse& response) {
            if (response.isSuccess() && !response.data.isNull()) {
                QJsonObject dirObj = response.data.toJsonObject();
                Directory newDir = Directory::fromJson(dirObj);
                
                if (newDir.isValid()) {
                    directoryList.append(newDir);
                    addDirectoryToList(newDir);
                    hideCreateInput();
                    dirNameEdit->clear();
                }
            } else {
                QMessageBox::warning(this, "提示", "创建目录失败：" + response.message);
            }
        },
        [this](const QString& error) {
            QMessageBox::warning(this, "提示", "网络错误：" + error);
        }
    );
}

void DirectoryDialog::onCloseCreateInput()
{
    hideCreateInput();
    dirNameEdit->clear();
}

void DirectoryDialog::onConfirmUpload()
{
    if (m_selectedDirectoryId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择目录");
        return;
    }
    
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择文件",
        QString(),
        "文档文件 (*.txt *.doc *.docx *.pdf);;所有文件 (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 获取用户ID
    User currentUser = TokenManager::instance().getUser();
    QString userId = currentUser.id;
    
    if (userId.isEmpty()) {
        QMessageBox::warning(this, "提示", "无法获取用户信息");
        return;
    }
    
    // 准备上传文件
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "提示", "无法打开文件");
        delete file;
        return;
    }
    
    // 构建上传URL
    QString url = QString(Constants::BASE_URL + Constants::Endpoints::UPLOAD_DOC)
                      .arg(m_tenantId)
                      .arg(m_selectedDirectoryId);
    
    QNetworkRequest request(url);
    
    // 添加认证头
    QString token = TokenManager::instance().getToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    }
    
    // 添加用户ID头
    request.setRawHeader("X-User-Id", userId.toUtf8());
    
    // 创建 multipart 表单数据
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
                      QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                              .arg(QFileInfo(filePath).fileName())));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    
    multiPart->append(filePart);
    
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, [this, reply, manager]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject obj = doc.object();
            
            if (obj["status"].toString() == "SUCCESS") {
                QMessageBox::information(this, "提示", "上传成功");
                accept();
            } else {
                QMessageBox::warning(this, "提示", "上传失败：" + obj["message"].toString());
            }
        } else {
            QMessageBox::warning(this, "提示", "上传失败：" + reply->errorString());
        }
        
        reply->deleteLater();
        manager->deleteLater();
    });
}

void DirectoryDialog::showCreateInput()
{
    createInputWidget->setVisible(true);
    createDirBtn->setVisible(false);
    dirNameEdit->setFocus();
}

void DirectoryDialog::hideCreateInput()
{
    createInputWidget->setVisible(false);
    createDirBtn->setVisible(true);
}

void DirectoryDialog::updateConfirmButtonState()
{
    if (m_selectedDirectoryId.isEmpty()) {
        confirmBtn->setEnabled(false);
        confirmBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   border: none;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "}"
        ).arg(Colors::GRAY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL));
    } else {
        confirmBtn->setEnabled(true);
        confirmBtn->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   border: none;"
            "   border-radius: %2px;"
            "   font-size: %3px;"
            "}"
            "QPushButton:hover {"
            "   background-color: %4;"
            "}"
        ).arg(Colors::PRIMARY_COLOR.name())
         .arg(Dimens::BTN_HEIGHT / 2)
         .arg(Dimens::FONT_SIZE_NORMAL)
         .arg(Colors::PRIMARY_COLOR.lighter(110).name()));
    }
}

void DirectoryDialog::onCancelCreate()
{
    hideCreateInput();
    dirNameEdit->clear();
}

void DirectoryDialog::onDirectorySelected()
{
    updateConfirmButtonState();
}