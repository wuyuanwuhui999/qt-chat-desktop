#ifndef STYLES_H
#define STYLES_H

#include <QString>
#include "Colors.h"
#include "Dimens.h"

namespace Styles {
    // 通用按钮样式
    inline QString primaryButtonStyle() {
        return QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: white;"
            "   border: none;"
            "   border-radius: %2px;"
            "   padding: %3px;"
            "   font-size: %4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: %5;"
            "}"
            "QPushButton:pressed {"
            "   background-color: %6;"
            "}"
            "QPushButton:disabled {"
            "   background-color: %7;"
            "}"
        )
        .arg(Colors::PRIMARY_COLOR.name())
        .arg(Dimens::BTN_BORDER_RADIUS)
        .arg(Dimens::BTN_PADDING)
        .arg(Dimens::FONT_SIZE_NORMAL)
        .arg(Colors::PRIMARY_COLOR.lighter(110).name())
        .arg(Colors::PRIMARY_COLOR.darker(110).name())
        .arg(Colors::GRAY_COLOR.name());
    }
    
    // 输入框样式
    inline QString inputStyle() {
        return QString(
            "QLineEdit {"
            "   border: %1px solid %2;"
            "   border-radius: %3px;"
            "   padding: %4px;"
            "   background-color: %5;"
            "   font-size: %6px;"
            "}"
            "QLineEdit:focus {"
            "   border-color: %7;"
            "}"
        )
        .arg(Dimens::BORDER_SIZE)
        .arg(Colors::LINE_COLOR.name())
        .arg(Dimens::MODULE_BORDER_RADIUS)
        .arg(Dimens::SMALL_MARGIN)
        .arg(Colors::WHITE_BACKGROUND_COLOR.name())
        .arg(Dimens::FONT_SIZE_NORMAL)
        .arg(Colors::PRIMARY_COLOR.name());
    }
    
    // 模块背景样式
    inline QString moduleStyle() {
        return QString(
            "QWidget {"
            "   background-color: %1;"
            "   border-radius: %2px;"
            "}"
        )
        .arg(Colors::WHITE_COLOR.name())
        .arg(Dimens::MODULE_BORDER_RADIUS);
    }
}

#endif // STYLES_H