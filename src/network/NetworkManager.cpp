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

void NetworkManager::bindReply(QNetworkReply* reply,
                               const SuccessCallback& successCallback,
                               const ErrorCallback& errorCallback) {
    connect(reply, &QNetworkReply::finished, [reply, successCallback, errorCallback]() {
        const QByteArray body = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            ApiResponse response = NetworkManager::instance().parseResponse(body);

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
                // 后端出错时也会返回 ResultEntity（例如 401 -> {"status":"FAIL","msg":"无效的认证令牌"}），
                // 优先把里面的 msg 报出来，比 Qt 的 errorString 有信息量得多
                const ApiResponse response = NetworkManager::instance().parseResponse(body);
                const QString message = response.message.isEmpty() ? reply->errorString()
                                                                  : response.message;
                errorCallback(message);
            }
        }
        reply->deleteLater();
    });
}

void NetworkManager::get(const QString& endpoint,
                         const SuccessCallback& successCallback,
                         const ErrorCallback& errorCallback) {
    QUrl url(Constants::BASE_URL + endpoint);
    QNetworkRequest request(url);
    addAuthHeader(request);

    bindReply(manager->get(request), successCallback, errorCallback);
}

void NetworkManager::post(const QString& endpoint,
                          const QJsonObject& data,
                          const SuccessCallback& successCallback,
                          const ErrorCallback& errorCallback) {
    QUrl url(Constants::BASE_URL + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);

    const QByteArray postData = QJsonDocument(data).toJson();

    bindReply(manager->post(request, postData), successCallback, errorCallback);
}

void NetworkManager::put(const QString& endpoint,
                         const QJsonObject& data,
                         const SuccessCallback& successCallback,
                         const ErrorCallback& errorCallback) {
    QUrl url(Constants::BASE_URL + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);

    const QByteArray putData = QJsonDocument(data).toJson();

    bindReply(manager->put(request, putData), successCallback, errorCallback);
}

void NetworkManager::del(const QString& endpoint,
                         const SuccessCallback& successCallback,
                         const ErrorCallback& errorCallback) {
    QUrl url(Constants::BASE_URL + endpoint);
    QNetworkRequest request(url);
    addAuthHeader(request);

    bindReply(manager->deleteResource(request), successCallback, errorCallback);
}
