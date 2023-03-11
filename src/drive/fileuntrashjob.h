/*
 * This file is part of LibKGAPI library
 *
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "fileabstractmodifyjob.h"
#include "kgapidrive_export.h"

class QNetworkAccessManager;
class QNetworkRequest;

namespace KGAPI2
{

namespace Drive
{

class KGAPIDRIVE_EXPORT FileUntrashJob : public KGAPI2::Drive::FileAbstractModifyJob
{
    Q_OBJECT

public:
    explicit FileUntrashJob(const QString &fileId, const AccountPtr &account, QObject *parent = nullptr);
    explicit FileUntrashJob(const QStringList &filesIds, const AccountPtr &account, QObject *parent = nullptr);
    explicit FileUntrashJob(const FilePtr &file, const AccountPtr &account, QObject *parent = nullptr);
    explicit FileUntrashJob(const FilesList &files, const AccountPtr &account, QObject *parent = nullptr);
    ~FileUntrashJob() override;

protected:
    Q_REQUIRED_RESULT QUrl url(const QString &fileId) override;
    void dispatchRequest(QNetworkAccessManager *accessManager, const QNetworkRequest &request, const QByteArray &data, const QString &contentType) override;

private:
    class Private;
    Private *const d;
    friend class Private;
};

} // namespace Drive

} // namespace KGAPI2
