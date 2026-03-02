#ifndef COLORS_H
#define COLORS_H

#include <QColor>

namespace Colors {
    // 主要颜色
    const QColor PRIMARY_COLOR = QColor(255, 174, 0);        // 0xFFFFAE00
    const QColor SECONDARY_COLOR = QColor(33, 150, 243);     // 0xFF2196F3
    const QColor BACKGROUND_COLOR = QColor(239, 239, 239);   // 0xFFEFEFEF
    const QColor TEXT_COLOR = QColor(0, 0, 0);               // 0xFF000000
    const QColor DARK_BACKGROUND_COLOR = QColor(0, 0, 0);    // 0xFF000000
    const QColor DARK_TEXT_COLOR = QColor(255, 255, 255);    // 0xFFFFFFFF
    
    // 界面颜色
    const QColor PAGE_BACKGROUND_COLOR = QColor(239, 239, 239);  // 0xFFEFEFEF
    const QColor MODULE_BACKGROUND_COLOR = QColor(255, 255, 255); // 0xFFFFFFFF
    const QColor DISABLE_TEXT_COLOR = QColor(221, 221, 221);      // 0xFFDDDDDD
    const QColor TAB_COLOR_ACTIVE = QColor(255, 174, 0);          // 0xFFFFAE00
    const QColor SEARCH_INPUT_COLOR = QColor(239, 239, 239);      // 0xFFEFEFEF
    const QColor SEARCH_INPUT_PLACEHOLD = QColor(221, 221, 221);  // 0xFFDDDDDD
    const QColor LINE_COLOR = QColor(33, 150, 243);               // 0xFF2196F3
    const QColor BLACK_BACKGROUND_COLOR = QColor(0, 0, 0);        // 0xFF000000
    const QColor WHITE_BACKGROUND_COLOR = QColor(255, 255, 255);  // 0xFFFFFFFF
    const QColor SELECTED_COLOR = QColor(255, 154, 0);            // 0xFFFF9A00
    const QColor SUB_TITLE_COLOR = QColor(153, 153, 153);         // 0xFF999999
    const QColor WARN_COLOR = QColor(247, 69, 59);                // 0xFFF7453B
    const QColor BLUE_COLOR = QColor(62, 125, 155);               // 0xFF3E7D9B
    const QColor LINEAR_GRADIENT = QColor(51, 51, 51);            // 0xFF333333
    const QColor POP_BACKGROUND_COLOR = QColor(51, 51, 51);       // 0xFF333333
    const QColor POP_LINE_COLOR = QColor(68, 68, 68);             // 0xFF444444
    
    // 透明颜色
    const QColor TRANSPARENT = QColor(0, 0, 0, 0);                // 0x00000000
}

#endif // COLORS_H