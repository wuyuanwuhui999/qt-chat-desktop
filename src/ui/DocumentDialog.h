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
#include <QStackedWidget>
#include <QStringList>
#include <QHash>
#include <QMap>
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
    
    // 获取选中的文档ID列表（我的文档 + 公共文档）
    QStringList getSelectedDocumentIds() const;

private slots:
    void onDirectoryClicked(int index);
    void onDocumentCheckStateChanged(int index, bool checked);
    void onConfirmClicked();
    void onCancelClicked();
    void onMyDocTabClicked();
    void onPublicDocTabClicked();
    void onPublicDirectoryClicked(int index);

private:
    void setupUI();
    QWidget* createTabBar();
    QWidget* createMyDocPage();
    QWidget* createPublicDocPage();
    void updateTabStyles();
    void switchToTab(int index);

    // 我的文档（原有逻辑）
    void loadDirectoryList();
    void loadDocumentList(const QString& directoryId);
    void clearDirectoryList();
    void clearDocumentList();
    void updateConfirmButtonState();
    void addDirectoryToUI(const Directory& dir, int index);
    void addDocumentToUI(const Document& doc, int index);

    // 公共文档
    void loadPublicDocList();
    void buildPublicDocTree();
    void clearPublicDocTree();
    void addPublicDirectoryToUI(const QString& directoryName, int index);
    void addPublicDocumentToUI(const Document& doc, const QString& directoryName);
    void onPublicDocumentCheckStateChanged(const QString& docId, bool checked);

    // 文档操作（... 菜单）
    void showDocumentMenu(const Document& doc, const QPoint& globalPos);
    void openPermissionDialog(const Document& doc);
    void deleteDocument(const Document& doc);
    void removeDocumentFromUI(const QString& docId);

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

    // 页签
    QWidget* m_tabBar;
    QPushButton* m_myDocTab;
    QPushButton* m_publicDocTab;
    QStackedWidget* m_pageStack;
    QWidget* m_myDocPage;
    QWidget* m_publicDocPage;
    QFrame* m_tabLine;

    // 存储每个目录项的widget和箭头按钮
    QList<QWidget*> m_directoryWidgets;
    QList<QPushButton*> m_arrowButtons;
    QList<bool> m_directoryExpanded;  // 目录是否展开
    QMap<QString, QWidget*> m_documentContainers;  // directoryId -> 文档容器
    QHash<QString, QWidget*> m_documentItemWidgets;  // docId -> 文档行控件

    // 公共文档
    QScrollArea* m_publicScrollArea;
    QWidget* m_publicContainerWidget;
    QVBoxLayout* m_publicContainerLayout;
    QList<Document> m_publicDocList;
    QStringList m_publicDirectoryNames;                          // 去重后的目录名（按出现顺序）
    QHash<QString, QList<Document>> m_publicDocsByDir;            // 目录名 -> 文档
    QList<QWidget*> m_publicDirectoryWidgets;
    QList<QPushButton*> m_publicArrowButtons;
    QList<bool> m_publicDirectoryExpanded;
    QHash<QString, QWidget*> m_publicDocContainers;              // 目录名 -> 文档容器
    QMap<QString, bool> m_publicSelectedDocuments;                // docId -> checked
    bool m_publicLoaded;
};

#endif // DOCUMENTDIALOG_H
