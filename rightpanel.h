#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QVBoxLayout>

class RightPanel : public QWidget
{
    Q_OBJECT  // 👈 必须添加！

public:
    explicit RightPanel(QWidget *parent = nullptr);

private:
    QVBoxLayout *layout;
};

#endif // RIGHTPANEL_H
