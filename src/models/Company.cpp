#include "Company.h"

Company::Company() : status(0) {}

Company Company::fromJson(const QJsonObject& json) {
    Company company;
    if (json.contains("id") && !json["id"].isNull())
        company.id = json["id"].toString();
    if (json.contains("name") && !json["name"].isNull())
        company.name = json["name"].toString();
    if (json.contains("code") && !json["code"].isNull())
        company.code = json["code"].toString();
    if (json.contains("description") && !json["description"].isNull())
        company.description = json["description"].toString();
    if (json.contains("status") && !json["status"].isNull())
        company.status = json["status"].toInt();
    return company;
}

QJsonObject Company::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["code"] = code;
    json["description"] = description;
    json["status"] = status;
    return json;
}
