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

#ifndef KGAPI2_DRIVECHILDREFERENCEFETCHJOB_H
#define KGAPI2_DRIVECHILDREFERENCEFETCHJOB_H

#include <libkgapi2/fetchjob.h>
#include <libkgapi2/libkgapi2_export.h>

namespace KGAPI2
{

class LIBKGAPI2_EXPORT DriveChildReferenceFetchJob : public KGAPI2::FetchJob
{
    Q_OBJECT

  public:
    explicit DriveChildReferenceFetchJob(const QString &folderId,
                                         const AccountPtr &account,
                                         QObject *parent = 0);
    explicit DriveChildReferenceFetchJob(const QString &folderId,
                                         const QString &childId,
                                         const AccountPtr &account,
                                         QObject *parent = 0);
    virtual ~DriveChildReferenceFetchJob();

  protected:
    virtual void start();
    virtual KGAPI2::ObjectsList handleReplyWithItems(const QNetworkReply *reply,
                                                     const QByteArray &rawData);

  private:
    class Private;
    Private * const d;
    friend class Private;

};

}  //namespace KGAPI2

#endif // KGAPI2_DRIVECHILDREFERENCEFETCHJOB_H
