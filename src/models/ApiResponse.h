#ifndef APIRESPONSE_H
#define APIRESPONSE_H

#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>

class ApiResponse {
public:
    ApiResponse();

    static ApiResponse fromJson(const QJsonObject& json);

    QVariant data;
    QString token;
    QString status;  // "SUCCESS" 或 "FAIL"
    QString message;
    int total;       // 分页时使用

    bool isSuccess() const { return status == "SUCCESS"; }

    template<typename T>
    T getData() const {
        return data.value<T>();
    }
};

#endif // APIRESPONSE_H
