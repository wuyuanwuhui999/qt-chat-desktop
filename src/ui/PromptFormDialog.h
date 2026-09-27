#ifndef PROMPTFORMDIALOG_H
#define PROMPTFORMDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include "models/Prompt.h"

// 新增 / 编辑提示词对话框（两种模式共用同一套界面，只有标题和提交接口不同）
class PromptFormDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { Add, Update };

    explicit PromptFormDialog(Mode mode,
                              const QString& tenantId,
                              const Prompt& prompt = Prompt(),
                              QWidget* parent = nullptr);

    // 编辑模式下提交成功后返回更新后的提示词
    Prompt result() const { return m_prompt; }

private slots:
    void onConfirmClicked();
    void onTextChanged();

private:
    void setupUI();
    void updateConfirmButtonState();
    void submit();

    Mode m_mode;
    QString m_tenantId;
    Prompt m_prompt;   // Update 模式下为待编辑的提示词

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QWidget* m_card;
    QTextEdit* m_promptEdit;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
};

#endif // PROMPTFORMDIALOG_H
