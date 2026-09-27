// Constants.h (需要添加的部分)
#include <QList>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

namespace Constants {
    // API 基础URL
    const QString BASE_URL = "http://127.0.0.1:4000";
    
    // WebSocket 聊天URL
    const QString WEBSOCKET_CHAT_URL = "ws://127.0.0.1:3000/service/chat/ws/chat?token=Bearer %1";

    // 缓存键
    const QString TOKEN_KEY = "jwt_token";
    const QString USER_KEY = "user_data";
    const QString CURRENT_TENANT_ID_KEY = "current_tenant_id";
    const QString SELECTED_MODEL_ID_KEY = "selected_model_id";
    const QString CURRENT_COMPANY_ID_KEY = "current_company_id";
    const QString CURRENT_COMPANY_KEY = "current_company";
    const QString SYSTEM_PROMPT_PREFIX = "system_prompt_"; // 租户系统提示词前缀
    
    // 默认系统提示词
    const QString DEFAULT_SYSTEM_PROMPT = "你叫小吴同学，是一个无所不能的AI助手，上知天文下知地理，请用小吴同学的身份回答问题。";
    
    // API 端点
    namespace Endpoints {
        // 用户相关
        const QString GET_USER_DATA = "/service/user/getUserData";
        const QString PASSWORD_LOGIN = "/service/user/login";
        const QString SEND_EMAIL_CODE = "/service/user/sendEmailVertifyCode";
        const QString EMAIL_LOGIN = "/service/user/loginByEmail";
        
        // 公司相关
        const QString GET_COMPANY_LIST = "/service/company/getCompanyList";

        // 租户相关
        const QString GET_TENANT_LIST = "/service/tenant/getTenantList";
        
        // 聊天相关
        const QString GET_CHAT_HISTORY = "/service/chat/getChatHistory";
        const QString GET_MODEL_LIST = "/service/chat/getModelList";
        
        // 提示词相关
        const QString GET_DEFAULT_PROMPT_BY_TENANT_ID = "/service/prompt/getDefaultPromptByTenantId";

        const QString GET_DIRECTORY_LIST = "/service/chat/getDirectoryList";
        const QString CREATE_DIR = "/service/chat/createDir";
        const QString UPLOAD_DOC = "/service/chat/uploadDoc/%1/%2";  // 需要传入 tenantId 和 directoryId

        const QString GET_DOC_LIST_BY_DIR_ID = "/service/chat/getDocListByDirId";

    }

    // 接口地址拼接查询参数（自动 URL 编码；值为空的参数会被忽略）
    // 返回可以直接交给 NetworkManager 的相对地址
    inline QString withQuery(const QString& endpoint,
                             const QList<QPair<QString, QString>>& params) {
        QUrl url(BASE_URL + endpoint);
        QUrlQuery query;
        bool hasParam = false;
        for (const QPair<QString, QString>& param : params) {
            if (param.second.isEmpty()) continue;
            query.addQueryItem(param.first, param.second);
            hasParam = true;
        }
        if (hasParam) {
            url.setQuery(query);
        }
        return url.toString().remove(BASE_URL);
    }

    // 默认租户
    namespace DefaultTenant {
        const QString NAME = "私人空间";
        const QString CODE = "personal";
        const int STATUS = 1;
        const QString CREATED_BY = "system";
    }
}
