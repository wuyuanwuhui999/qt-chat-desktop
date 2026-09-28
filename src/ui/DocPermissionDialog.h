#ifndef DOCPERMISSIONDIALOG_H
#define DOCPERMISSIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include "models/Document.h"

// 编辑文档权限对话框
class DocPermissionDialog : public QDialog
{
    Q_OBJECT

public:
    // 权限可选值（顺序与后端约定一致）
    static QStringList permissionValues();
    // 权限值 -> 显示文案，例如 tenant -> 租户内公开
    static QString permissionLabel(const QString& value);

    explicit DocPermissionDialog(const Document& doc, QWidget* parent = nullptr);

    // 确定后选中的权限值
    QString selectedPermission() const { return m_selectedPermission; }

private slots:
    void onConfirmClicked();

private:
    void setupUI();

    Document m_doc;
    QString m_selectedPermission;

    QVBoxLayout* m_mainLayout;
    QWidget* m_contentWidget;
    QComboBox* m_permissionCombo;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
};

#endif // DOCPERMISSIONDIALOG_H
