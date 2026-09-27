#ifndef PROMPTDIALOG_H
#define PROMPTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QHash>
#include <QList>
#include "models/Prompt.h"

// 提示词库对话框：搜索 + 分页列表 + 删除/编辑/使用 + 取消/确定
class PromptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PromptDialog(const QString& tenantId,
                          const QString& currentPromptId = QString(),
                          QWidget* parent = nullptr);

    // 当前“使用中”的提示词 id（点击确定后由外层读取）
    QString selectedPromptId() const { return m_usedPromptId; }

private slots:
    void onSearchTextChanged();
    void onScrollValueChanged(int value);
    void onRefreshClicked();
    void onAddClicked();
    void onConfirmClicked();

private:
    void setupUI();
    QWidget* createHeaderArea();
    QWidget* createContentArea();

    void loadPrompts(int pageNum, bool append);
    void addPromptItem(const Prompt& prompt);
    void clearList();
    void removePromptItem(const QString& promptId);
    void updateSeparators();
    void updateConfirmButtonState();
    void applyUsedState();
    void showEmptyHint(const QString& text);

    void openAddDialog();
    void openEditDialog(const Prompt& prompt);
    void deletePrompt(const Prompt& prompt);
    void toggleUse(const Prompt& prompt);

    QString m_tenantId;
    QString m_usedPromptId;   // 正在使用的提示词 id

    int m_pageNum;
    int m_pageSize;
    int m_total;
    bool m_isLoading;
    bool m_hasMore;
    QList<Prompt> m_prompts;
    QHash<QString, QWidget*> m_itemWidgets;   // promptId -> 条目控件

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QPushButton* m_refreshBtn;
    QPushButton* m_addBtn;

    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QLineEdit* m_searchEdit;

    QWidget* m_listCard;
    QScrollArea* m_scrollArea;
    QWidget* m_listContainer;
    QVBoxLayout* m_listLayout;

    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;

    QTimer* m_searchDebounce;
    int m_textWidth;   // 条目文本可用宽度（用于最多三行 + 省略号）
};

#endif // PROMPTDIALOG_H
