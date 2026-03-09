// TokenManager.h
#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QString>
#include <QSettings>
#include "models/User.h"

class TokenManager {
public:
    static TokenManager& instance();

    void saveToken(const QString& token);
    QString getToken() const;
    void clearToken();

    void saveUser(const User& user);
    User getUser() const;
    void clearUser();

    bool hasValidToken() const;
    
    // 添加公共方法来访问设置
    void setValue(const QString& key, const QVariant& value);
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

private:
    TokenManager();
    ~TokenManager();

    QSettings settings;  // 保持私有
    QString m_token;
    User m_user;
};

#endif // TOKENMANAGER_H