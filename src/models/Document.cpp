#include "Document.h"

Document::Document() {}

Document Document::fromJson(const QJsonObject& json) {
    Document doc;
    if (json.contains("id")) doc.id = json["id"].toString();
    if (json.contains("tenantId")) doc.tenantId = json["tenantId"].toString();
    if (json.contains("directoryId")) doc.directoryId = json["directoryId"].toString();
    if (json.contains("directoryName")) doc.directoryName = json["directoryName"].toString();
    if (json.contains("name")) doc.name = json["name"].toString();
    if (json.contains("ext")) doc.ext = json["ext"].toString();
    if (json.contains("userId")) doc.userId = json["userId"].toString();
    if (json.contains("createTime") && !json["createTime"].isNull())
        doc.createTime = json["createTime"].toString();
    if (json.contains("updateTime") && !json["updateTime"].isNull())
        doc.updateTime = json["updateTime"].toString();
    return doc;
}

QJsonObject Document::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["tenantId"] = tenantId;
    json["directoryId"] = directoryId;
    json["directoryName"] = directoryName;
    json["name"] = name;
    json["ext"] = ext;
    json["userId"] = userId;
    json["createTime"] = createTime;
    json["updateTime"] = updateTime;
    return json;
}