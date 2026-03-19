#ifndef DIRECTORYDIALOG_H
#define DIRECTORYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>
#include "theme/Colors.h"
#include "theme/Dimens.h"
#include "models/ApiResponse.h"

// 目录实体结构体
struct DirectoryEntity {
    QString id;
    QString userId;
    QString directory;
    QString tenantId;
    QString updateTime;
    QString createTime;
    
    static DirectoryEntity fromJson(const QJsonObject& json) {
        DirectoryEntity dir;
        if (json.contains("id")) dir.id = json["id"].toString();
        if (json.contains("userId")) dir.userId = json["userId"].toString();
        if (json.contains("directory")) dir.directory = json["directory"].toString();
        if (json.contains("tenantId")) dir.tenantId = json["tenantId"].toString();
        if (json.contains("updateTime") && !json["updateTime"].isNull())
            dir.updateTime = json["updateTime"].toString();
        if (json.contains("createTime") && !json["createTime"].isNull())
            dir.createTime = json["createTime"].toString();
        return dir;
    }
    
    bool isValid() const { return !id.isEmpty(); }
};

Q_DECLARE_METATYPE(DirectoryEntity)

class DirectoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirectoryDialog(const QString& tenantId, QWidget *parent = nullptr);
    ~DirectoryDialog();

private slots:
    void loadDirectoryList();
    void onCreateDirClicked();
    void onConfirmCreateDir();
    void onDirectorySelected();
    void onConfirmUpload();
    void onCancelCreate();
    void onCloseCreateInput();  // 新增：关闭创建输入框

private:
    void setupUI();
    void addDirectoryToList(const DirectoryEntity& dir);
    void clearDirectoryList();
    void showCreateInput();
    void hideCreateInput();
    void updateConfirmButtonState();
    void updateCreateInputButtonsStyle();  // 新增：更新创建输入框按钮样式
    
    QString m_tenantId;
    QString m_selectedDirectoryId;
    QString m_selectedDirectoryName;
    
    // UI组件
    QVBoxLayout* mainLayout;
    QListWidget* directoryListWidget;
    QPushButton* createDirBtn;
    QPushButton* confirmBtn;
    QPushButton* cancelBtn;
    
    // 创建目录相关
    QWidget* createInputWidget;
    QHBoxLayout* createInputLayout;
    QLineEdit* dirNameEdit;
    QPushButton* confirmCreateBtn;  // 确认按钮（主色调圆形）
    QPushButton* closeCreateBtn;    // 新增：关闭按钮（灰色圆形）
    
    QButtonGroup* radioGroup;
    QList<DirectoryEntity> directoryList;
};

#endif // DIRECTORYDIALOG_H