#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

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
    QString createTime;
    QString updateTime;
    
    static Document fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
    
    bool isValid() const { return !id.isEmpty(); }
};

#endif // DOCUMENT_H