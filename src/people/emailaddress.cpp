/*
 * SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 * SPDX-License-Identifier: LGPL-3.0-only
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */

#include "emailaddress.h"

#include "fieldmetadata.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSharedData>

#include <algorithm>

namespace KGAPI2::People
{

struct EmailAddressDefinition
{
    FieldMetadata metadata;
    QString value;
    QString type;
    QString formattedType;
    QString displayName;
};

class EmailAddress::Private : public QSharedData
{
public:
    explicit Private() = default;
    Private(const Private &) = default;
    Private(Private &&) noexcept = delete;
    Private &operator=(const Private &) = delete;
    Private &operator=(Private &&) noexcept = delete;
    ~Private() = default;

    bool operator==(const Private &other) const
    {
        return value == other.value && metadata == other.metadata && type == other.type && displayName == other.displayName
            && formattedType == other.formattedType;
    }

    bool operator!=(const Private &other) const
    {
        return !(*this == other);
    }

    QString value{};
    FieldMetadata metadata{};
    QString type{};
    QString displayName{};
    QString formattedType{};
};

EmailAddress::EmailAddress()
    : d(new Private)
{
}

EmailAddress::EmailAddress(const EmailAddressDefinition &definition)
    : d(new Private)
{
    d->metadata = definition.metadata;
    d->value = definition.value;
    d->type = definition.type;
    d->formattedType = definition.formattedType;
    d->displayName = definition.displayName;
}

EmailAddress::EmailAddress(const EmailAddress &) = default;
EmailAddress::EmailAddress(EmailAddress &&) noexcept = default;
EmailAddress &EmailAddress::operator=(const EmailAddress &) = default;
EmailAddress &EmailAddress::operator=(EmailAddress &&) noexcept = default;
EmailAddress::~EmailAddress() = default;

bool EmailAddress::operator==(const EmailAddress &other) const
{
    return *d == *other.d;
}

bool EmailAddress::operator!=(const EmailAddress &other) const
{
    return !(*this == other);
}

QString EmailAddress::value() const
{
    return d->value;
}

void EmailAddress::setValue(const QString &value)
{
    d->value = value;
}
FieldMetadata EmailAddress::metadata() const
{
    return d->metadata;
}

void EmailAddress::setMetadata(const FieldMetadata &value)
{
    d->metadata = value;
}
QString EmailAddress::type() const
{
    return d->type;
}

void EmailAddress::setType(const QString &value)
{
    d->type = value;
}
QString EmailAddress::displayName() const
{
    return d->displayName;
}

void EmailAddress::setDisplayName(const QString &value)
{
    d->displayName = value;
}
QString EmailAddress::formattedType() const
{
    return d->formattedType;
}

EmailAddress EmailAddress::fromJSON(const QJsonObject &obj)
{
    if(!obj.isEmpty()) {
        EmailAddressDefinition definition;

        const auto metadata = obj.value(QStringLiteral("metadata")).toObject();
        definition.metadata = FieldMetadata::fromJSON(metadata);
        definition.value = obj.value(QStringLiteral("value")).toString();
        definition.type = obj.value(QStringLiteral("type")).toString();
        definition.formattedType = obj.value(QStringLiteral("formattedType")).toString();
        definition.displayName = obj.value(QStringLiteral("displayName")).toString();

        return EmailAddress(definition);
    }

    return EmailAddress();
}

QVector<EmailAddress> EmailAddress::fromJSONArray(const QJsonArray& data)
{
    QVector<EmailAddress> emailAddresses;

    for(const auto emailAddress : data) {
        if(emailAddress.isObject()) {
            const auto objectifiedEmailAddress = emailAddress.toObject();
            emailAddresses.append(fromJSON(objectifiedEmailAddress));
        }
    }

    return emailAddresses;
}


QJsonValue EmailAddress::toJSON() const
{
    QJsonObject obj;

    obj.insert(QStringView{u"value"}, d->value);
    obj.insert(QStringView{u"metadata"}, d->metadata.toJSON());
    obj.insert(QStringView{u"type"}, d->type);
    obj.insert(QStringView{u"displayName"}, d->displayName);
    obj.insert(QStringView{u"formattedType"}, d->formattedType);
    return obj;
}

} // namespace KGAPI2::People
