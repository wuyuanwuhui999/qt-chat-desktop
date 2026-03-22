#include "Directory.h"

Directory::Directory() {}

Directory Directory::fromJson(const QJsonObject& json) {
    Directory dir;
    if (json.contains("id")) dir.id = json["id"].toString();
    if (json.contains("userId")) dir.userId = json["userId"].toString();
    if (json.contains("directory")) dir.directory = json["directory"].toString();
    if (json.contains("tenantId")) dir.tenantId = json["tenantId"].toString();
    if (json.contains("updateTime") && !json["updateTime"].isNull())
        dir.updateTime = json["updateTime"].toString();
    if (json.contains("createTime") && !json["createTime"].isNull())
        dir.createTime = json["createTime"].toString();
    return dir;
}

QJsonObject Directory::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["userId"] = userId;
    json["directory"] = directory;
    json["tenantId"] = tenantId;
    json["updateTime"] = updateTime;
    json["createTime"] = createTime;
    return json;
}