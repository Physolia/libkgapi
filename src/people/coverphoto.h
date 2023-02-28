/*
 * SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
 * SPDX-FileCopyrightText: 2022 Claudio Cambra <claudio.cambra@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 * SPDX-License-Identifier: LGPL-3.0-only
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QSharedDataPointer>
#include "kgapipeople_export.h"

#include <QString>

#include <optional>

class QJsonObject;
class QJsonValue;
class QJsonArray;

namespace KGAPI2::People
{
class FieldMetadata;

/**
 * A person's cover photo. A large image shown on the person's profile page that
 * represents who they are or what they care about.
 *
 * @see https://developers.google.com/people/api/rest/v1/people#coverphoto
 * @since 5.19.0
 **/
class KGAPIPEOPLE_EXPORT CoverPhoto
{
public:
    /** Constructs a new CoverPhoto **/
    explicit CoverPhoto();
    CoverPhoto(const CoverPhoto &);
    CoverPhoto(CoverPhoto &&) noexcept;
    CoverPhoto &operator=(const CoverPhoto &);
    CoverPhoto &operator=(CoverPhoto &&) noexcept;
    /** Destructor. **/
    ~CoverPhoto();

    bool operator==(const CoverPhoto &) const;
    bool operator!=(const CoverPhoto &) const;

    Q_REQUIRED_RESULT static CoverPhoto fromJSON(const QJsonObject &obj);
    Q_REQUIRED_RESULT static QVector<CoverPhoto> fromJSONArray(const QJsonArray &data);
    Q_REQUIRED_RESULT QJsonValue toJSON() const;

    /** True if the cover photo is the default cover photo; false if the cover photo is a user-provided cover photo. **/
    Q_REQUIRED_RESULT bool isDefault() const;
    /** Sets value of the isDefault property. **/
    void setIsDefault(const bool &value);

    /** Metadata about the cover photo. **/
    Q_REQUIRED_RESULT FieldMetadata metadata() const;
    /** Sets value of the metadata property. **/
    void setMetadata(const FieldMetadata &value);

    /** The URL of the cover photo. **/
    Q_REQUIRED_RESULT QString url() const;
    /** Sets value of the url property. **/
    void setUrl(const QString &value);

private:
    class Private;
    QSharedDataPointer<Private> d;
}; // CoverPhoto

} // namespace KGAPI2::People
