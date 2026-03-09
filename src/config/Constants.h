// Constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
    // API 基础URL
    const QString BASE_URL = "http://192.168.73.8:3000";

    // 缓存键
    const QString TOKEN_KEY = "jwt_token";
    const QString USER_KEY = "user_data";
    const QString CURRENT_TENANT_ID_KEY = "current_tenant_id";

    // API 端点
    namespace Endpoints {
        // 用户相关
        const QString GET_USER_DATA = "/service/user/getUserData";
        const QString PASSWORD_LOGIN = "/service/user/login";
        const QString SEND_EMAIL_CODE = "/service/user/sendEmailVertifyCode";
        const QString EMAIL_LOGIN = "/service/user/loginByEmail";
        
        // 租户相关
        const QString GET_USER_TENANT_LIST = "/service/tenant/getUserTenantList";
        
        // 聊天相关
        const QString GET_CHAT_HISTORY = "/service/chat/getChatHistory";
        // 其他端点...
    }

    // 默认租户
    namespace DefaultTenant {
        const QString NAME = "私人空间";
        const QString CODE = "personal";
        const int STATUS = 1;
        const QString CREATED_BY = "system";
    }

}

#endif // CONSTANTS_H