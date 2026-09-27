#ifndef STYLES_H
#define STYLES_H

#include <QString>
#include "Colors.h"
#include "Dimens.h"

namespace Styles {
    // 通用按钮样式
    // 规范：高度 BTN_HEIGHT，圆角 BTN_HEIGHT/2，主色背景，白字，
    //       内边距 PAGE_PADDING，禁用态 GRAY_COLOR
    inline QString primaryButtonStyle() {
        return QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: none;"
            "   border-radius: %3px;"
            "   padding: %4px;"
            "   font-size: %5px;"
            "}"
            "QPushButton:hover {"
            "   background-color: %6;"
            "}"
            "QPushButton:pressed {"
            "   background-color: %7;"
            "}"
            "QPushButton:disabled {"
            "   background-color: %8;"
            "   color: %2;"
            "}"
        )
        .arg(Colors::PRIMARY_COLOR.name())
        .arg(Colors::WHITE_COLOR.name())
        .arg(Dimens::BTN_HEIGHT / 2)
        .arg(Dimens::PAGE_PADDING)
        .arg(Dimens::FONT_SIZE_NORMAL)
        .arg(Colors::PRIMARY_COLOR.lighter(110).name())
        .arg(Colors::PRIMARY_COLOR.darker(110).name())
        .arg(Colors::GRAY_COLOR.name());
    }

    // 取消（次要）按钮样式
    // 规范：背景透明，边框与文字 GRAY_COLOR
    inline QString secondaryButtonStyle() {
        return QString(
            "QPushButton {"
            "   background-color: transparent;"
            "   color: %1;"
            "   border: 1px solid %1;"
            "   border-radius: %2px;"
            "   padding: %3px;"
            "   font-size: %4px;"
            "}"
            "QPushButton:hover {"
            "   border-color: %5;"
            "   color: %5;"
            "}"
        )
        .arg(Colors::GRAY_COLOR.name())
        .arg(Dimens::BTN_HEIGHT / 2)
        .arg(Dimens::PAGE_PADDING)
        .arg(Dimens::FONT_SIZE_NORMAL)
        .arg(Colors::PRIMARY_COLOR.name());
    }

    // 单行输入框样式
    // 规范：高度 INPUT_HEIGHT，圆角 INPUT_HEIGHT/2，内边距 PAGE_PADDING
    inline QString inputStyle() {
        return QString(
            "QLineEdit {"
            "   border: %1px solid %2;"
            "   border-radius: %3px;"
            "   padding: 0 %4px;"
            "   background-color: %5;"
            "   font-size: %6px;"
            "}"
            "QLineEdit:focus {"
            "   border-color: %7;"
            "}"
        )
        .arg(Dimens::BORDER_SIZE)
        .arg(Colors::GRAY_COLOR.name())
        .arg(Dimens::INPUT_HEIGHT / 2)
        .arg(Dimens::PAGE_PADDING)
        .arg(Colors::WHITE_COLOR.name())
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
