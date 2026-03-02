#ifndef USER_H
#define USER_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

class User {
public:
    User();
    
    QString id;
    QString userAccount;
    QDateTime createDate;
    QDateTime updateDate;
    QString username;
    QString telephone;
    QString email;
    QString avatar;
    QDateTime birthday;
    int sex;            // 0:未知, 1:男, 2:女
    QString role;
    QString password;   // 通常不保存
    QString sign;
    QString region;
    int disabled;       // 0:启用, 1:禁用
    int permission;     // 权限级别
    
    static User fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
    
    bool isValid() const { return !id.isEmpty(); }
};

#endif // USER_H