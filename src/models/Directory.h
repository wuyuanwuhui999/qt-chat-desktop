#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

class Directory {
public:
    Directory();
    
    QString id;
    QString userId;
    QString directory;
    QString tenantId;
    QString updateTime;
    QString createTime;
    
    static Directory fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
    
    bool isValid() const { return !id.isEmpty(); }
};

#endif // DIRECTORY_H