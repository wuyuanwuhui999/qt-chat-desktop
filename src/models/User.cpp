#include "User.h"
#include <QJsonArray>

User::User() : sex(0), disabled(0), permission(0) {}

User User::fromJson(const QJsonObject& json) {
    User user;
    
    if (json.contains("id")) user.id = json["id"].toString();
    if (json.contains("userAccount")) user.userAccount = json["userAccount"].toString();
    if (json.contains("createDate")) user.createDate = QDateTime::fromString(json["createDate"].toString(), Qt::ISODate);
    if (json.contains("updateDate")) user.updateDate = QDateTime::fromString(json["updateDate"].toString(), Qt::ISODate);
    if (json.contains("username")) user.username = json["username"].toString();
    if (json.contains("telephone")) user.telephone = json["telephone"].toString();
    if (json.contains("email")) user.email = json["email"].toString();
    if (json.contains("avatar")) user.avatar = json["avatar"].toString();
    if (json.contains("birthday")) user.birthday = QDateTime::fromString(json["birthday"].toString(), Qt::ISODate);
    if (json.contains("sex")) user.sex = json["sex"].toInt();
    if (json.contains("role")) user.role = json["role"].toString();
    if (json.contains("password")) user.password = json["password"].toString();
    if (json.contains("sign")) user.sign = json["sign"].toString();
    if (json.contains("region")) user.region = json["region"].toString();
    if (json.contains("disabled")) user.disabled = json["disabled"].toInt();
    if (json.contains("permission")) user.permission = json["permission"].toInt();
    
    return user;
}

QJsonObject User::toJson() const {
    QJsonObject json;
    
    json["id"] = id;
    json["userAccount"] = userAccount;
    json["createDate"] = createDate.toString(Qt::ISODate);
    json["updateDate"] = updateDate.toString(Qt::ISODate);
    json["username"] = username;
    json["telephone"] = telephone;
    json["email"] = email;
    json["avatar"] = avatar;
    json["birthday"] = birthday.toString(Qt::ISODate);
    json["sex"] = sex;
    json["role"] = role;
    json["sign"] = sign;
    json["region"] = region;
    json["disabled"] = disabled;
    json["permission"] = permission;
    
    return json;
}