#ifndef PROMPT_H
#define PROMPT_H

#include <QString>
#include <QJsonObject>

class Prompt {
public:
    Prompt();

    QString id;
    QString prompt;
    QString tenantId;
    QString userId;
    QString createTime;

    static Prompt fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

    bool isValid() const { return !id.isEmpty(); }
};

#endif // PROMPT_H
