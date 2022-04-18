/*
 *  SPDX-FileCopyrightText: 2022 Claudio Cambra <claudio.cambra@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "peopleservice.h"
#include "person.h"
#include "contactgroup.h"
#include "address.h"
#include "agerangetype.h"
#include "utils.h"
#include "debug.h"

//#include <QDomDocument>
//#include <QDomElement>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

/* Qt::escape() */
#include <QTextDocument>

namespace KGAPI2
{

namespace PeopleService
{

namespace Private
{
static const QUrl GoogleApisUrl(QStringLiteral("https://people.googleapis.com"));
static const QString PeopleBasePath(QStringLiteral("/v1/people"));
static const QString AllPersonFields(QStringLiteral("addresses,ageRanges,biographies,birthdays,calendarUrls,clientData,coverPhotos,emailAdresses,events,externalIds,genders,imClients,interests,locales,locations,memberships,metadata,miscKeywords,names,nicknames,occupations,organizations,phoneNumbers,photos,relations,sipAddresses,skills,urls,userDefined"));
}

People::PersonPtr JSONToPerson(const QByteArray& jsonData)
{
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if(document.isObject()) {
        const auto objectifiedDocument = document.object();
        const auto resourceName = objectifiedDocument.value(QStringLiteral("resourceName")).toString();

        return resourceName.startsWith(QStringLiteral("people")) ? People::Person::fromJSON(objectifiedDocument) : People::PersonPtr();
    }

    return People::PersonPtr();
}

QUrl fetchAllContactsUrl()
{
    QUrl url(Private::GoogleApisUrl);
    const QString path = Private::PeopleBasePath % QStringLiteral("/me/connections");
    url.setPath(path);

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("alt"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("personFields"), Private::AllPersonFields);

    url.setQuery(query);
    return url;
}

// https://developers.google.com/people/api/rest/v1/people/searchContacts
QUrl fetchContactUrl(const QString &fetchQuery, const QString &readMask)
{
    QUrl url(Private::GoogleApisUrl);
    const QString path = Private::PeopleBasePath % QStringLiteral(":searchContacts");
    url.setPath(path);

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("alt"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("query"), fetchQuery);
    query.addQueryItem(QStringLiteral("readMask"), readMask);

    url.setQuery(query);
    return url;
}

QUrl createContactUrl()
{
    QUrl url(Private::GoogleApisUrl);
    const QString path = Private::PeopleBasePath % QStringLiteral(":createContact");
    url.setPath(path);
    return url;
}

QUrl updateContactUrl(QString &resourceName, QString &personFields)
{
    QUrl url(Private::GoogleApisUrl);
    url.setPath(QStringLiteral("/v1/") % resourceName % QStringLiteral(":createContact"));

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("updatePersonFields"), personFields);

    url.setQuery(query);
    return url;
}

QUrl deleteContactUrl(const QString &resourceName)
{
    QUrl url(Private::GoogleApisUrl);
    url.setPath(QStringLiteral("/v1/") % resourceName % QStringLiteral(":deleteContact"));
    return url;
}

ObjectsList parseConnectionsJSONFeed(FeedData feedData, const QByteArray &jsonFeed)
{
    ObjectsList output;
    const auto document = QJsonDocument::fromJson(jsonFeed);

    if(!document.isObject()) {
        return {};
    }

    const auto rootObject = document.object();
    const auto connections = rootObject.value(QStringLiteral("connections")).toArray();
    for(const auto connection : connections) {
        output.append(People::Person::fromJSON(connection.toObject()));
    }

    feedData.totalResults = rootObject.value(QStringLiteral("totalItems")).toInt();

    if(rootObject.contains(QStringLiteral("nextPageToken"))) {
        auto url = fetchAllContactsUrl();

        QUrlQuery query(url);
        query.addQueryItem(QStringLiteral("pageToken"), rootObject.value(QStringLiteral("nextPageToken")).toString());

        url.setQuery(query);
        feedData.nextPageUrl = url;
    }

    return output;
}

}

}
