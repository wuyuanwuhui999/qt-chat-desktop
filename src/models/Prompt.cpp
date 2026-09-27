#include "Prompt.h"

Prompt::Prompt() {}

Prompt Prompt::fromJson(const QJsonObject& json) {
    Prompt prompt;
    if (json.contains("id") && !json["id"].isNull())
        prompt.id = json["id"].toString();
    if (json.contains("prompt") && !json["prompt"].isNull())
        prompt.prompt = json["prompt"].toString();
    if (json.contains("tenantId") && !json["tenantId"].isNull())
        prompt.tenantId = json["tenantId"].toString();
    if (json.contains("userId") && !json["userId"].isNull())
        prompt.userId = json["userId"].toString();
    if (json.contains("createTime") && !json["createTime"].isNull())
        prompt.createTime = json["createTime"].toString();
    return prompt;
}

QJsonObject Prompt::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["prompt"] = prompt;
    json["tenantId"] = tenantId;
    json["userId"] = userId;
    json["createTime"] = createTime;
    return json;
}
