/*
    SPDX-FileCopyrightText: 2022 Claudio Cambra <claudio.cambra@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "types.h"
#include "kgapipeople_export.h"

#include <QUrl>

namespace KGAPI2
{

/**
 * @brief Additional methods for implementing support for Google People service
 *
 * You should never need to use these methods, unless implementing your own Job
 */

namespace PeopleService
{

    KGAPIPEOPLE_EXPORT People::PersonPtr JSONtoPerson(const QByteArray &jsonData);

    //KGAPIPEOPLE_EXPORT People::ContactGroupPtr JSONToContactGroup(const QByteArray &jsonData);

    KGAPIPEOPLE_EXPORT QUrl fetchAllContactsUrl();
    KGAPIPEOPLE_EXPORT QUrl fetchContactUrl(const QString &fetchQuery, const QString &readMask);
    KGAPIPEOPLE_EXPORT QUrl createContactUrl();
    KGAPIPEOPLE_EXPORT QUrl updateContactUrl(QString &resourceName, QString &personFields);
    KGAPIPEOPLE_EXPORT QUrl deleteContactUrl(QString &resourceName);
    KGAPIPEOPLE_EXPORT ObjectsList parseConnectionsJSONFeed(FeedData &feedData, const QByteArray &jsonFeed);
}

}
