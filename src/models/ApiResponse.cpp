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
    
    if (json.contains("message")) {
        response.message = json["message"].toString();
    }
    
    if (json.contains("total")) {
        response.total = json["total"].toInt();
    }
    
    return response;
}