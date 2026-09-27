#ifndef COMPANY_H
#define COMPANY_H

#include <QString>
#include <QJsonObject>

class Company {
public:
    Company();

    QString id;
    QString name;
    QString code;
    QString description;
    int status;

    static Company fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

    bool isValid() const { return !id.isEmpty(); }
};

#endif // COMPANY_H
