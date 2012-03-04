/*
    libKGoogle - Services - Calendar
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


#include "calendar.h"
#include "accessmanager.h"
#include "objects/calendar.h"
#include "objects/event.h"

#ifdef WITH_KCAL
  #include <kcal/event.h>
  #include <kcal/attendee.h>
  #include <kcal/alarm.h>
  #include <kcal/recurrence.h>
  #include <kcal/recurrencerule.h>
  #include <kcal/icalformat.h>
  using namespace KCal;
#else
  #include <kcalcore/event.h>
  #include <kcalcore/attendee.h>
  #include <kcalcore/alarm.h>
  #include <kcalcore/recurrence.h>
  #include <kcalcore/recurrencerule.h>
  #include <kcalcore/icalformat.h>
  using namespace KCalCore;
#endif

#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <kcalcore/alarm.h>
#include <ksystemtimezone.h>
#include <klocale.h>

#include <qdebug.h>

using namespace KGoogle;


QUrl Services::Calendar::ScopeUrl("https://www.googleapis.com/auth/calendar");

/********** PUBLIC JSON INTERFACE ************/

KGoogle::Object* Services::Calendar::JSONToObject(const QByteArray& jsonData)
{
  QJson::Parser parser;

  QVariantMap object = parser.parse(jsonData).toMap();

  if ((object["kind"] == "calendar#calendarListEntry") || (object["kind"] == "calendar#calendar"))
    return JSONToCalendar(object);
  else if (object["kind"] == "calendar#event")
    return JSONToEvent(object);

  return 0;
}

QByteArray Services::Calendar::objectToJSON(KGoogle::Object* object)
{
  QVariantMap map;

  if (dynamic_cast< const Objects::Calendar* >(object)) {
    map = calendarToJSON(object);
  } else if (dynamic_cast< const Objects::Event* >(object)) {
    map = eventToJSON(object);
  }

  QJson::Serializer serializer;
  return serializer.serialize(map);
}

QList< KGoogle::Object* > Services::Calendar::parseJSONFeed(const QByteArray& jsonFeed, FeedData* feedData)
{
  QJson::Parser parser;

  QVariantMap data = parser.parse(jsonFeed).toMap();

  QList< KGoogle::Object* > list;

  if (data["kind"] == "calendar#calendarList") {
    list = parseCalendarJSONFeed(data["items"].toList());

    if (feedData && data.contains("nextPageToken")) {
      feedData->nextLink = fetchCalendarsUrl();
      feedData->nextLink.addQueryItem("pageToken", data["nextPageToken"].toString());
    }

  } else if (data["kind"] == "calendar#events") {
    list = parseEventJSONFeed(data["items"].toList());

    if (feedData && data.contains("nextPageToken") && data.contains("id")) {
      feedData->nextLink = fetchEventsUrl(data["id"].toString());
      feedData->nextLink.addQueryItem("pageToken", data["nextPageToken"].toString());
    }
  }

  return list;
}


/************* PUBLIC XML INTERFACE ***********/

KGoogle::Object* Services::Calendar::XMLToObject(const QByteArray& xmlData)
{
  Q_UNUSED (xmlData);

  return 0;
}

QByteArray Services::Calendar::objectToXML(KGoogle::Object* object)
{
  Q_UNUSED (object);

  return QByteArray();
}

QList< KGoogle::Object* > Services::Calendar::parseXMLFeed(const QByteArray& xmlFeed, FeedData* feedData)
{
  Q_UNUSED (xmlFeed);
  Q_UNUSED (feedData);

  return QList< KGoogle::Object* >();
}


/************* URLS **************/

const QUrl& Services::Calendar::scopeUrl() const
{
  return Services::Calendar::ScopeUrl;
}

QUrl Services::Calendar::fetchCalendarsUrl()
{
  return QUrl("https://www.googleapis.com/calendar/v3/users/me/calendarList");
}

QUrl Services::Calendar::fetchCalendarUrl(const QString& calendarID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/users/me/calendarList/");
  ba.append(QUrl::toPercentEncoding(calendarID));
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::updateCalendarUrl(const QString &calendarID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID));
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::createCalendarUrl()
{
  return QUrl("https://www.googleapis.com/calendar/v3/calendars");
}

QUrl Services::Calendar::removeCalendarUrl(const QString& calendarID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID));
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::fetchEventsUrl(const QString& calendarID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID));
  ba.append("/events");
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::fetchEventUrl(const QString& calendarID, const QString& eventID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID)).append("/events/").append(eventID.toLatin1());
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::updateEventUrl(const QString& calendarID, const QString& eventID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID)).append("/events/").append(eventID.toLatin1());
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::createEventUrl(const QString& calendarID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID)).append("/events");
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::removeEventUrl(const QString& calendarID, const QString& eventID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(calendarID)).append("/events/").append(eventID.toLatin1());
  return QUrl::fromEncoded(ba);
}

QUrl Services::Calendar::moveEventUrl (const QString& sourceCalendar, const QString& destCalendar, const QString& eventID)
{
  QByteArray ba("https://www.googleapis.com/calendar/v3/calendars/");
  ba.append(QUrl::toPercentEncoding(sourceCalendar))
    .append(QString("/events/").toLatin1())
    .append(eventID.toLatin1())
    .append(QString("?destination=").toLatin1())
    .append(destCalendar.toLatin1());
  return QUrl::fromEncoded(ba);
}


QString Services::Calendar::protocolVersion() const
{
  return "3";
}

bool Services::Calendar::supportsJSONRead(QString* urlParam)
{
  return true;

  Q_UNUSED(urlParam)
}

bool Services::Calendar::supportsJSONWrite(QString* urlParam)
{
  return true;

  Q_UNUSED(urlParam)
}




/******** PRIVATE METHODS ************/
KGoogle::Object* Services::Calendar::JSONToCalendar(const QVariantMap& calendar)
{
  Objects::Calendar *object = new Objects::Calendar();

  QString id = QUrl::fromPercentEncoding(calendar["id"].toByteArray());
  object->setUid(id);
  object->setEtag(calendar["etag"].toString());
  object->setTitle(calendar["summary"].toString());
  object->setDetails(calendar["description"].toString());
  object->setLocation(calendar["location"].toString());
  object->setTimezone(calendar["timeZone"].toString());

  if ((calendar["accessRole"].toString() == "writer") || (calendar["acessRole"].toString() == "owner"))
    object->setEditable(true);
  else
    object->setEditable(false);

  QVariantList reminders = calendar["defaultReminders"].toList();
  foreach (const QVariant &r, reminders) {
    QVariantMap reminder = r.toMap();

    AlarmPtr alarm(new Alarm(0));
    if (reminder["method"].toString() == "email")
      alarm->setType(Alarm::Email);
    else if (reminder["method"].toString() == "popup")
      alarm->setType(Alarm::Display);
    else
      alarm->setType(KCalCore::Alarm::Invalid);

    alarm->setStartOffset(Duration(reminder["minutes"].toInt()*(-60)));

    object->addDefaultReminer(alarm);
  }

  return dynamic_cast< KGoogle::Object* >(object);
}

QVariantMap Services::Calendar::calendarToJSON(KGoogle::Object* calendar)
{
  QVariantMap output, entry;
  Objects::Calendar *object = static_cast< Objects::Calendar* >(calendar);

  if (!object->uid().isEmpty())
    entry["id"] = object->uid();

  entry["summary"] = object->title();
    entry["description"] = object->details();
  entry["location"] = object->location();
  if (!object->timezone().isEmpty())
    entry["timeZone"] = object->timezone();

  return entry;
}

QList< KGoogle::Object* > Services::Calendar::parseCalendarJSONFeed(const QVariantList& feed)
{
  QList< KGoogle::Object* > output;

  foreach (QVariant i, feed) {
    output.append(JSONToCalendar(i.toMap()));
  }

  return output;
}

KGoogle::Object* Services::Calendar::JSONToEvent(const QVariantMap& event)
{
  Objects::Event *object = new Objects::Event();

  /* ID */
  object->setUid(QUrl::fromPercentEncoding(event["id"].toByteArray()));

  /* ETAG */
  object->setEtag(event["etag"].toString());

  /* Status */
  if (event["status"].toString() == "confirmed") {
    object->setStatus(Incidence::StatusConfirmed);
  } else if (event["status"].toString() == "cancelled") {
    object->setStatus(Incidence::StatusCanceled);
    object->setDeleted(true);
  } else if (event["status"].toString() == "tentative") {
    object->setStatus(Incidence::StatusTentative);
  } else {
    object->setStatus(Incidence::StatusNone);
  }

  /* Canceled instance of recurring event. Set ID of the instance to match ID of the event */
  if (event.contains("recurringEventId") && object->deleted()) {
    object->setUid(QUrl::fromPercentEncoding(event["recurringEventId"].toByteArray()));
  }

  /* Created */
  object->setCreated(AccessManager::RFC3339StringToDate(event["created"].toString()));

  /* Last updated */
  object->setLastModified(AccessManager::RFC3339StringToDate(event["updated"].toString()));

  /* Summary */
  object->setSummary(event["summary"].toString());

  /* Description */
  object->setDescription(event["description"].toString());

  /* Location */
  object->setLocation(event["location"].toString());

  /* Organizer */
  PersonPtr organizer(new Person);
  QVariantMap organizerData = event["organizer"].toMap();
  organizer->setName(organizerData["displayName"].toString());
  organizer->setEmail(organizerData["email"].toString());
#ifdef WITH_KCAL
  object->setOrganizer(*organizer);
#else
  object->setOrganizer(organizer);
#endif

  /* Start date */
  QVariantMap startData = event["start"].toMap();
  KDateTime dtStart;
  if (startData.contains("date")) {
    dtStart = KDateTime::fromString(startData["date"].toString(), KDateTime::ISODate);
    object->setAllDay(true);
  } else if (startData.contains("dateTime")) {
    dtStart = AccessManager::RFC3339StringToDate(startData["dateTime"].toString());
  }
  if (startData.contains("timeZone")) {
    KTimeZone tz(startData["timeZone"].toString());
    dtStart.setTimeSpec(KDateTime::Spec(tz));
  }
  object->setDtStart(dtStart);

  /* End date */
  QVariantMap endData = event["end"].toMap();
  KDateTime dtEnd;
  if (endData.contains("date")) {
    dtEnd = KDateTime::fromString(endData["date"].toString(), KDateTime::ISODate);
    /* For Google, all-day events starts on Monday and ends on Tuesday, 
     * while in KDE, it both starts and ends on Monday. */
    dtEnd = dtEnd.addDays(-1);
    object->setAllDay(true);
  } else if (endData.contains("dateTime")) {
    dtEnd = AccessManager::RFC3339StringToDate(endData["dateTime"].toString());
  }
  if (endData.contains("timeZone")) {
    KTimeZone tz(endData["timeZone"].toString());
    dtEnd.setTimeSpec(KDateTime::Spec(tz));
  }
  object->setDtEnd(dtEnd);

  /* Transparency */
  if (event["transparency"].toString() == "transparent") {
    object->setTransparency(Event::Transparent);
  } else { /* Assume opaque as default transparency */
    object->setTransparency(Event::Opaque);
  }

  /* Attendees */
  QVariantList attendees = event["attendees"].toList();
  foreach (const QVariant &a, attendees) {
    QVariantMap att = a.toMap();
    AttendeePtr attendee(
      new Attendee(att["displayName"].toString(),
                   att["email"].toString()));

    if (att["responseStatus"].toString() == "accepted")
      attendee->setStatus(Attendee::Accepted);
    else if (att["responseStatus"].toString() == "declined")
      attendee->setStatus(Attendee::Declined);
    else if (att["responseStatus"].toString() == "tentative")
      attendee->setStatus(Attendee::Tentative);
    else
      attendee->setStatus(Attendee::NeedsAction);

    if (att["optional"].toBool())
      attendee->setRole(KCalCore::Attendee::OptParticipant);

    object->addAttendee(attendee, true);
  }

  /* Recurrence */
  QStringList recrs = event["recurrence"].toStringList();
  foreach (const QString &rec, recrs) {
    ICalFormat format;
    if (rec.left(5) == "RRULE") {
      RecurrenceRule *recurrenceRule = new RecurrenceRule();
      format.fromString(recurrenceRule, rec.mid(6));
      object->recurrence()->addRRule(recurrenceRule);
    } else if (rec.left(6) == "EXRULE") {
      RecurrenceRule *recurrenceRule = new RecurrenceRule();
      format.fromString(recurrenceRule, rec.mid(7));
      object->recurrence()->addExRule(recurrenceRule);
    } else if (rec.left(6) == "EXDATE") {
      DateList exdates = parseRDate(rec);
      object->recurrence()->setExDates(exdates);
    } else if (rec.left(5) == "RDATE") {
      DateList rdates = parseRDate(rec);
      object->recurrence()->setRDates(rdates);
    }
  }

  QVariantMap reminders = event["reminders"].toMap();
  if (reminders.contains("useDefault") && reminders["useDefault"].toBool())
    object->setUseDefaultReminders(true);
  else
    object->setUseDefaultReminders(false);

  QVariantList overrides = reminders["overrides"].toList();
  foreach (const QVariant &r, overrides) {
    QVariantMap override = r.toMap();
    AlarmPtr alarm(new Alarm(object));
    alarm->setTime(object->dtStart());

    if (override["method"].toString() == "alert")
      alarm->setType(Alarm::Display);
    else if (override["method"].toString() == "email")
      alarm->setType(Alarm::Email);
    else {
      alarm->setType(KCalCore::Alarm::Invalid);
      continue;
    }

    alarm->setStartOffset(Duration(override["minutes"].toInt()*(-60)));
    alarm->setEnabled(true);
    object->addAlarm(alarm);
  }

  /* Extended properties */
  QVariantMap extendedProperties = event["extendedProperties"].toMap();
  QVariantMap privateProperties = extendedProperties["private"].toMap();

  foreach (const QString &key, privateProperties.keys()) {

    if (key == "categories") {
      object->setCategories(privateProperties.value(key).toString());
    }
  }

  QVariantMap sharedProperties = extendedProperties["shared"].toMap();
  foreach (const QString &key, sharedProperties.keys()) {

    if (key == "categories") {
      object->setCategories(sharedProperties.value(key).toString());
    }
  }

  return dynamic_cast< KGoogle::Object* >(object);
}

QVariantMap Services::Calendar::eventToJSON(KGoogle::Object* event)
{
  Objects::Event *object = static_cast<Objects::Event*>(event);
  QVariantMap output, data;

  /* Type */
  data["type"] = "calendar#event";

  /* ID */
  if (!object->uid().isEmpty())
    data["id"] = object->uid();

  /* Status */
  if (object->status() == Incidence::StatusConfirmed)
    data["status"] = "confirmed";
  else if (object->status() == Incidence::StatusCanceled)
    data["status"] = "canceled";
  else if (object->status() == Incidence::StatusTentative)
    data["status"] = "tentative";

  /* Summary */
  data["summary"] = object->summary();

  /* Description */
  data["description"] = object->description();

  /* Location */
  data["location"] = object->location();

  /* Organizer */
  if (!object->organizer()->isEmpty()) {
      QVariantMap organizer;
      organizer["displayName"] = object->organizer()->fullName();
      organizer["email"] = object->organizer()->email();
      data["organizer"] = organizer;
  }

  /* Recurrence */
  QVariantList recurrence;
  ICalFormat format;
  foreach (RecurrenceRule *rRule, object->recurrence()->rRules())
    recurrence << format.toString(rRule).remove("\r\n");

  foreach (RecurrenceRule *rRule, object->recurrence()->exRules())
    recurrence << format.toString(rRule).remove("\r\n");

  QStringList dates;
  foreach (const QDate &rDate, object->recurrence()->rDates())
    dates << rDate.toString("yyyyMMdd");

  if (!dates.isEmpty())
    recurrence << "RDATE;VALUE=DATA:" + dates.join(",");

  dates.clear();
  foreach (const QDate &exDate, object->recurrence()->exDates())
    dates << exDate.toString("yyyyMMdd");

  if (!dates.isEmpty())
    recurrence << "EXDATE;VALUE=DATE:" + dates.join(",");

  if (!recurrence.isEmpty())
    data["recurrence"] = recurrence;

  /* Start */
  QVariantMap start;
  if (object->allDay()) {
    start["date"] = object->dtStart().toString("%Y-%m-%d");
    QString tz = object->dtStart().timeZone().name();
    if (!recurrence.isEmpty() && tz.isEmpty())
      tz = KTimeZone::utc().name();
    if (!tz.isEmpty())
      start["timeZone"] = tz;
  } else {
    start["dateTime"] = AccessManager::dateToRFC3339String(object->dtStart());
  }
  data["start"] = start;

  /* End */
  QVariantMap end;
  if (object->allDay()) {
    /* For Google, all-day events starts on Monday and ends on Tuesday, 
     * while in KDE, it both starts and ends on Monday. */
    KDateTime dtEnd = object->dtEnd().addDays(1);
    end["date"] = dtEnd.toString("%Y-%m-%d");
    QString tz = object->dtEnd().timeZone().name();
    if (!recurrence.isEmpty() && tz.isEmpty())
      tz = KTimeZone::utc().name();
    if (!tz.isEmpty())
      end["timeZone"] = tz;
  } else {
    end["dateTime"] = AccessManager::dateToRFC3339String(object->dtEnd());
  }
  data["end"] = end;

  /* Transparency */
  if (object->transparency() == Event::Transparent)
    data["transparency"] = "transparent";
  else
    data["transparency"] = "opaque";

  /* Attendees */
  QVariantList atts;
  foreach (const AttendeePtr attee, object->attendees()) {
    QVariantMap att;

    att["displayName"] = attee->name();
    att["email"] = attee->email();

    if (attee->status() == Attendee::Accepted)
      att["responseStatus"] = "accepted";
    else if (attee->status() == Attendee::Declined)
      att["responseStatus"] = "declined";
    else if (attee->status() == Attendee::Tentative)
      att["responseStatus"] = "tentative";
    else
      att["responseStatus"] = "needsAction";

    if (attee->role() == Attendee::OptParticipant)
      att["optional"] = true;

    atts.append(att);
  }

  if (!atts.isEmpty())
    data["attendees"] = atts;

  /* Reminders */
  QVariantList overrides;
  foreach (AlarmPtr alarm, object->alarms()) {
    QVariantMap override;

    if (alarm->type() == Alarm::Display)
      override["method"] = "popup";
    else if (alarm->type() == Alarm::Email)
      override["method"] = "email";
    else
      continue;

    override["minutes"] = (int) (alarm->startOffset().asSeconds() / -60);

    overrides << override;
  }

  if (!overrides.isEmpty())
    data["reminders"] = overrides;

  /* Store categories */
  if (!object->categories().isEmpty()) {
    QVariantMap extendedProperties;
    QVariantMap sharedProperties;
    sharedProperties["categories"] = object->categoriesStr();
    extendedProperties["shared"] = sharedProperties;
    data["extendedProperties"] = extendedProperties;
  }

  /* TODO: Implement support for additional features:
   * http://code.google.com/apis/gdata/docs/2.0/elements.html
   */

  return data;
}

QList< KGoogle::Object* > Services::Calendar::parseEventJSONFeed(const QVariantList& feed)
{
  QList< KGoogle::Object* > output;

  foreach (QVariant i, feed) {
    output.append(JSONToEvent(i.toMap()));
  }

  return output;
}


DateList Services::Calendar::parseRDate (const QString& rule) const
{
  DateList list;
  QString value;
  KTimeZone tz;

  QString left = rule.left(rule.indexOf(":"));
  QStringList params = left.split(";");
  foreach (const QString &param, params) {
    if (param.startsWith("VALUE")) {
      value = param.mid(param.indexOf("=") + 1);
    } else if (param.startsWith("TZID")) {
      QString tzname = param.mid(param.indexOf("=") + 1);
      tz = KSystemTimeZones::zone(tzname);
    }
  }

  QString datesStr = rule.mid(rule.lastIndexOf(":") + 1);
  QStringList dates = datesStr.split(",");
  foreach (QString date, dates) {
    QDate dt;

    if (value == "DATE") {
      dt = QDate::fromString(date, "yyyyMMdd");
    } else if (value == "PERIOD") {
      QString start = date.left(date.indexOf("/"));
      KDateTime kdt = AccessManager::RFC3339StringToDate(start);
      if (tz.isValid())
        kdt.setTimeSpec(tz);

      dt = kdt.date();
    } else {
      KDateTime kdt = AccessManager::RFC3339StringToDate(date);
      if (tz.isValid())
        kdt.setTimeSpec(tz);

      dt = kdt.date();
    }

    list << dt;
  }

  return list;
}