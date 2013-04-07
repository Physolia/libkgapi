/*
 * This file is part of LibKGAPI library
 *
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "taskcreatejob.h"
#include "tasksservice.h"
#include "account.h"
#include "debug.h"
#include "utils.h"
#include "task.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <KDE/KLocalizedString>

using namespace KGAPI2;

class TaskCreateJob::Private
{
  public:
    TasksList tasks;
    QString taskListId;
    QString parentId;
};

TaskCreateJob::TaskCreateJob(const TaskPtr& task, const QString& taskListId,
                             const AccountPtr& account, QObject* parent):
    CreateJob(account, parent),
    d(new Private)
{
    d->tasks << task;
    d->taskListId = taskListId;
}

TaskCreateJob::TaskCreateJob(const TasksList& tasks, const QString& taskListId,
                             const AccountPtr& account, QObject* parent):
    CreateJob(account, parent),
    d(new Private)
{
    d->tasks = tasks;
    d->taskListId = taskListId;
}

TaskCreateJob::~TaskCreateJob()
{
    delete d;
}

QString TaskCreateJob::parentItem() const
{
    return d->parentId;
}

void TaskCreateJob::setParentItem(const QString &parentId)
{
    if (isRunning()) {
        KGAPIWarning() << "Can't modify parentItem property when job is running!";
        return;
    }

    d->parentId = parentId;
}

void TaskCreateJob::start()
{
   if (d->tasks.isEmpty()) {
        emitFinished();
        return;
    }

    const TaskPtr task = d->tasks.takeFirst();

    QUrl url = TasksService::createTaskUrl(d->taskListId);
    if (!d->parentId.isEmpty()) {
        url.addQueryItem(QLatin1String("parent"), d->parentId);
    }
    QNetworkRequest request;
    request.setRawHeader("Authorization", "Bearer " + account()->accessToken().toLatin1());
    request.setUrl(url);

    const QByteArray rawData = TasksService::taskToJSON(task);

    QStringList headers;
    Q_FOREACH(const QByteArray &str, request.rawHeaderList()) {
        headers << QLatin1String(str) + QLatin1String(": ") + QLatin1String(request.rawHeader(str));
    }
    KGAPIDebugRawData() << headers;

    enqueueRequest(request, rawData, QLatin1String("application/json"));
}

ObjectsList TaskCreateJob::handleReplyWithItems(const QNetworkReply *reply, const QByteArray& rawData)
{
    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    ContentType ct = Utils::stringToContentType(contentType);
    ObjectsList items;
    if (ct == KGAPI2::JSON) {
        items << TasksService::JSONToTask(rawData).dynamicCast<Object>();
    } else {
        setError(KGAPI2::InvalidResponse);
        setErrorString(i18n("Invalid response content type"));
        emitFinished();
    }

    // Enqueue next item or finish
    start();

    return items;
}

#include "taskcreatejob.moc"
