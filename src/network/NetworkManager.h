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
    static NetworkManager& instance();
    
    void get(const QString& endpoint, 
             const std::function<void(const ApiResponse&)>& successCallback,
             const std::function<void(const QString&)>& errorCallback);
    
    void post(const QString& endpoint, 
              const QJsonObject& data,
              const std::function<void(const ApiResponse&)>& successCallback,
              const std::function<void(const QString&)>& errorCallback);
    
    void setAuthToken(const QString& token);
    QString getAuthToken() const;

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    
    QNetworkAccessManager* manager;
    QString authToken;
    
    void addAuthHeader(QNetworkRequest& request);
    ApiResponse parseResponse(const QByteArray& data);
};

#endif // NETWORKMANAGER_H