// Constants.h (需要添加的部分)
namespace Constants {
    // API 基础URL
    const QString BASE_URL = "http://127.0.0.1:3000";
    
    // WebSocket 聊天URL
    const QString WEBSOCKET_CHAT_URL = "ws://127.0.0.1:3000/service/chat/ws/chat?token=Bearer %1";

    // 缓存键
    const QString TOKEN_KEY = "jwt_token";
    const QString USER_KEY = "user_data";
    const QString CURRENT_TENANT_ID_KEY = "current_tenant_id";
    const QString SELECTED_MODEL_ID_KEY = "selected_model_id";
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
        
        // 租户相关
        const QString GET_USER_TENANT_LIST = "/service/tenant/getUserTenantList";
        
        // 聊天相关
        const QString GET_CHAT_HISTORY = "/service/chat/getChatHistory";
        const QString GET_MODEL_LIST = "/service/chat/getModelList";
        
        // 提示词相关
        const QString GET_DEFAULT_PROMPT_BY_TENANT_ID = "/service/prompt/getDefaultPromptByTenantId";
    }

    // 默认租户
    namespace DefaultTenant {
        const QString NAME = "私人空间";
        const QString CODE = "personal";
        const int STATUS = 1;
        const QString CREATED_BY = "system";
    }
}
