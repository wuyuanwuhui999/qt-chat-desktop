#ifndef COLORS_H
#define COLORS_H

#include <QColor>

namespace Colors {
    // 主要颜色
    const QColor PRIMARY_COLOR = QColor(255, 174, 0);   
    const QColor SUB_TITLE_COLOR = QColor(153, 153, 153); 
    const QColor SECONDARY_COLOR = QColor(33, 150, 243);
    const QColor BACKGROUND_COLOR = QColor(239, 239, 239);
    const QColor TEXT_COLOR = QColor(0, 0, 0);
    
    // 界面颜色
    const QColor PAGE_BACKGROUND_COLOR = QColor(239, 239, 239);
    const QColor WHITE_COLOR = QColor(255, 255, 255);
    const QColor GRAY_COLOR = QColor(221, 221, 221);
    const QColor SEARCH_INPUT_COLOR = QColor(239, 239, 239);
    const QColor SEARCH_INPUT_PLACEHOLD = QColor(221, 221, 221);
    const QColor LINE_COLOR = QColor(33, 150, 243);
    const QColor WARN_COLOR = QColor(247, 69, 59);
    
    // 透明颜色
    const QColor TRANSPARENT = QColor(0, 0, 0, 0);
}

#endif // COLORS_H