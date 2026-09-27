#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <functional>
#include "models/ApiResponse.h"

class NetworkManager : public QObject {
    Q_OBJECT

public:
    // 接口回调类型（所有接口统一使用 ApiResponse）
    using SuccessCallback = std::function<void(const ApiResponse&)>;
    using ErrorCallback = std::function<void(const QString&)>;

    static NetworkManager& instance();
    
    void get(const QString& endpoint,
             const SuccessCallback& successCallback,
             const ErrorCallback& errorCallback);
    
    void post(const QString& endpoint,
              const QJsonObject& data,
              const SuccessCallback& successCallback,
              const ErrorCallback& errorCallback);

    void put(const QString& endpoint,
             const QJsonObject& data,
             const SuccessCallback& successCallback,
             const ErrorCallback& errorCallback);

    void del(const QString& endpoint,
             const SuccessCallback& successCallback,
             const ErrorCallback& errorCallback);
    
    void setAuthToken(const QString& token);
    QString getAuthToken() const;

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    
    QNetworkAccessManager* manager;
    QString authToken;
    
    void addAuthHeader(QNetworkRequest& request);
    ApiResponse parseResponse(const QByteArray& data);

    // get/post/put/del 共用的回复处理：解析 ApiResponse、刷新 token、分发回调
    void bindReply(QNetworkReply* reply,
                   const SuccessCallback& successCallback,
                   const ErrorCallback& errorCallback);
};

#endif // NETWORKMANAGER_H
