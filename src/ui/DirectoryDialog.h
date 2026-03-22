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
#include "models/Directory.h"

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
    void onCloseCreateInput();

private:
    void setupUI();
    void addDirectoryToList(const Directory& dir);
    void clearDirectoryList();
    void showCreateInput();
    void hideCreateInput();
    void updateConfirmButtonState();
    void updateCreateInputButtonsStyle();
    
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
    QPushButton* confirmCreateBtn;
    QPushButton* closeCreateBtn;
    
    QButtonGroup* radioGroup;
    QList<Directory> directoryList;
};

#endif // DIRECTORYDIALOG_H