#include "LeftPanel.h"
#include <QLabel>

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #888;");

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 示例内容（后续可替换为列表、按钮等）
    QLabel *label = new QLabel("Left Panel (10%~50%)", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("color: white; font-size: 16px;");
    layout->addWidget(label);

    setLayout(layout);
}
