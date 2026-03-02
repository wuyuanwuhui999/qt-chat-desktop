#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
// API 基础URL
const QString BASE_URL = "http://192.168.73.8";

// 缓存键
const QString TOKEN_KEY = "jwt_token";
const QString USER_KEY = "user_data";

// API 端点
namespace Endpoints {
    const QString GET_USER_DATA = "/service/user/getUserData";
    // 其他端点...
    }
}

#endif // CONSTANTS_H
