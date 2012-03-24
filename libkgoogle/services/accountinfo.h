/*
    libKGoogle - Services - AccountInfo
    Copyright (C) 2011  Dan Vratil <dan@progdan.cz>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef LIBKGOOGLE_SERVICES_ACCOUNTINFO_H
#define LIBKGOOGLE_SERVICES_ACCOUNTINFO_H

#include <libkgoogle/service.h>
#include <libkgoogle/libkgoogle_export.h>

namespace KGoogle
{

class Object;

namespace Services
{

class LIBKGOOGLE_EXPORT AccountInfo : public KGoogle::Service
{
  public:
    static QUrl ScopeUrl;
    static QUrl EmailScopeUrl;

    /**
     * Implements KGoogle::Service::JSONToObject().
     */
    KGoogle::Object *JSONToObject(const QByteArray &jsonData);

    /**
     * Service version 1 is supported now.
     */
    QString protocolVersion() const;

    /**
     * The AccountInfo provides read-only access, so serializer is not implemented.
     *
     * This method does nothing.
     */
    QByteArray objectToJSON(KGoogle::Object *object);

    /**
     * Returns scope URL for AccountInfo service.
     */
    const QUrl& scopeUrl() const;

    /**
     * Returns KGoogle::Request::Fetch URL.
     *
     * https://www.googleapis.com/oauth2/v1/userinfo
     */
    static QUrl fetchUrl();

    /**
     * The AccountInfo always returns informations about single account, never feed.
     *
     * This method does nothing.
     */
    QList< KGoogle::Object * > parseJSONFeed(const QByteArray &jsonFeed, KGoogle::FeedData& feedData);

    /**
     * The AccountInfo service supports only JSON format.
     *
     * This method does nothing.
     */
    QList< KGoogle::Object * > parseXMLFeed(const QByteArray &xmlFeed, KGoogle::FeedData& feedData);

    /**
     * The AccountInfo service supports only JSON format.
     *
     * This method does nothing.
     */
    QByteArray objectToXML(KGoogle::Object *object);

    /**
     * The AccountInfo service supports only JSON format.
     *
     * This method does nothing.
     */
    KGoogle::Object *XMLToObject(const QByteArray &xmlData);

};

} /* namespace Services */

} /* namespace KGoogle */

#endif // LIBKGOOGLE_SERVICES_ACCOUNTINFO_H
