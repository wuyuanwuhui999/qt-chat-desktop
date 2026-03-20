#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

class Document {
public:
    Document();
    
    QString id;
    QString tenantId;
    QString directoryId;
    QString directoryName;
    QString name;
    QString ext;
    QString userId;
    QDateTime createTime;
    QDateTime updateTime;
    
    static Document fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
    
    bool isValid() const { return !id.isEmpty(); }
};

#endif // DOCUMENT_H