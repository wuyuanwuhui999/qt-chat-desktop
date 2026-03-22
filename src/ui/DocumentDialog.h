// DocumentDialog.h
#ifndef DOCUMENTDIALOG_H
#define DOCUMENTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include <QButtonGroup>
#include <QCheckBox>
#include "models/Directory.h"
#include "models/Document.h"
#include "theme/Colors.h"
#include "theme/Dimens.h"

class DocumentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentDialog(const QString& tenantId, QWidget *parent = nullptr);
    ~DocumentDialog();
    
    // 获取选中的文档ID列表
    QStringList getSelectedDocumentIds() const;

private slots:
    void onDirectoryClicked(int index);
    void onDocumentCheckStateChanged(int index, bool checked);
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();
    void loadDirectoryList();
    void loadDocumentList(const QString& directoryId);
    void clearDirectoryList();
    void clearDocumentList();
    void updateConfirmButtonState();
    void addDirectoryToUI(const Directory& dir, int index);
    void addDocumentToUI(const Document& doc, int index);
    
    QString m_tenantId;
    QString m_currentDirectoryId;
    QList<Directory> m_directoryList;
    QList<Document> m_documentList;
    QMap<int, bool> m_selectedDocuments;  // index -> checked
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QVBoxLayout* m_containerLayout;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
    
    // 存储每个目录项的widget和箭头按钮
    QList<QWidget*> m_directoryWidgets;
    QList<QPushButton*> m_arrowButtons;
    QList<bool> m_directoryExpanded;  // 目录是否展开
    QMap<QString, QWidget*> m_documentContainers;  // directoryId -> 文档容器
};

#endif // DOCUMENTDIALOG_H