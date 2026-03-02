#include "TokenManager.h"
#include "config/Constants.h"
#include <QJsonDocument>

TokenManager::TokenManager() : settings("YourCompany", "ChatApp") {
    m_token = settings.value(Constants::TOKEN_KEY).toString();
    
    QByteArray userData = settings.value(Constants::USER_KEY).toByteArray();
    if (!userData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(userData);
        m_user = User::fromJson(doc.object());
    }
}

TokenManager::~TokenManager() {}

TokenManager& TokenManager::instance() {
    static TokenManager instance;
    return instance;
}

void TokenManager::saveToken(const QString& token) {
    m_token = token;
    settings.setValue(Constants::TOKEN_KEY, token);
}

QString TokenManager::getToken() const {
    return m_token;
}

void TokenManager::clearToken() {
    m_token.clear();
    settings.remove(Constants::TOKEN_KEY);
}

void TokenManager::saveUser(const User& user) {
    m_user = user;
    QJsonDocument doc(user.toJson());
    settings.setValue(Constants::USER_KEY, doc.toJson());
}

User TokenManager::getUser() const {
    return m_user;
}

void TokenManager::clearUser() {
    m_user = User();
    settings.remove(Constants::USER_KEY);
}

bool TokenManager::hasValidToken() const {
    return !m_token.isEmpty();
}