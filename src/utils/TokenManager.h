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

private:
    TokenManager();
    ~TokenManager();

    QSettings settings;
    QString m_token;
    User m_user;
};

#endif // TOKENMANAGER_H
