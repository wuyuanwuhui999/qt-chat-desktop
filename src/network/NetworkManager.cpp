#include "NetworkManager.h"
#include "config/Constants.h"
#include "utils/TokenManager.h"
#include <QUrl>
#include <QJsonDocument>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
}

NetworkManager::~NetworkManager() {}

NetworkManager& NetworkManager::instance() {
    static NetworkManager instance;
    return instance;
}

void NetworkManager::setAuthToken(const QString& token) {
    authToken = token;
}

QString NetworkManager::getAuthToken() const {
    return authToken;
}

void NetworkManager::addAuthHeader(QNetworkRequest& request) {
    if (!authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(authToken).toUtf8());
    }
}

ApiResponse NetworkManager::parseResponse(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return ApiResponse::fromJson(doc.object());
}

void NetworkManager::get(const QString& endpoint,
                         const std::function<void(const ApiResponse&)>& successCallback,
                         const std::function<void(const QString&)>& errorCallback) {
    QUrl url(Constants::BASE_URL + endpoint);
    QNetworkRequest request(url);
    addAuthHeader(request);
    
    QNetworkReply* reply = manager->get(request);
    
    connect(reply, &QNetworkReply::finished, [reply, successCallback, errorCallback]() {
        if (reply->error() == QNetworkReply::NoError) {
            ApiResponse response = NetworkManager::instance().parseResponse(reply->readAll());
            
            // 如果返回了新token，更新缓存
            if (!response.token.isEmpty()) {
                TokenManager::instance().saveToken(response.token);
                NetworkManager::instance().setAuthToken(response.token);
            }
            
            if (successCallback) {
                successCallback(response);
            }
        } else {
            if (errorCallback) {
                errorCallback(reply->errorString());
            }
        }
        reply->deleteLater();
    });
}

void NetworkManager::post(const QString& endpoint,
                          const QJsonObject& data,
                          const std::function<void(const ApiResponse&)>& successCallback,
                          const std::function<void(const QString&)>& errorCallback) {
    QUrl url(Constants::BASE_URL + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);
    
    QJsonDocument doc(data);
    QByteArray postData = doc.toJson();
    
    QNetworkReply* reply = manager->post(request, postData);
    
    connect(reply, &QNetworkReply::finished, [reply, successCallback, errorCallback]() {
        if (reply->error() == QNetworkReply::NoError) {
            ApiResponse response = NetworkManager::instance().parseResponse(reply->readAll());
            
            if (!response.token.isEmpty()) {
                TokenManager::instance().saveToken(response.token);
                NetworkManager::instance().setAuthToken(response.token);
            }
            
            if (successCallback) {
                successCallback(response);
            }
        } else {
            if (errorCallback) {
                errorCallback(reply->errorString());
            }
        }
        reply->deleteLater();
    });
}