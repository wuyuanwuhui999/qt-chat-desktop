#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QVBoxLayout>

class LeftPanel : public QWidget
{
    Q_OBJECT  // 👈 必须添加！

public:
    explicit LeftPanel(QWidget *parent = nullptr);

private:
    QVBoxLayout *layout;
};

#endif // LEFTPANEL_H
