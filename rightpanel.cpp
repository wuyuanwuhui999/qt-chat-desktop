#include "RightPanel.h"
#include <QLabel>

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: white;");

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 示例内容
    QLabel *label = new QLabel("Right Panel (Flexible)", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("color: black; font-size: 16px;");
    layout->addWidget(label);

    setLayout(layout);
}
