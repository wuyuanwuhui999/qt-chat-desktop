#include "ApiResponse.h"

ApiResponse::ApiResponse() : total(0) {}

ApiResponse ApiResponse::fromJson(const QJsonObject& json) {
    ApiResponse response;
    
    if (json.contains("data") && !json["data"].isNull()) {
        response.data = json["data"].toVariant();
    }
    
    if (json.contains("token")) {
        response.token = json["token"].toString();
    }
    
    if (json.contains("status")) {
        response.status = json["status"].toString();
    }
    
    // 后端字段名为 message，但部分接口返回的是 msg，两个都兼容
    if (json.contains("message") && !json["message"].isNull()) {
        response.message = json["message"].toString();
    } else if (json.contains("msg") && !json["msg"].isNull()) {
        response.message = json["msg"].toString();
    }
    
    if (json.contains("total")) {
        response.total = json["total"].toInt();
    }
    
    return response;
}