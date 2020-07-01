/*
 * This file is part of LibKGAPI
 *
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LIBKGAPI2_AUTHJOB_H
#define LIBKGAPI2_AUTHJOB_H

#include "job.h"
#include "kgapicore_export.h"

class QWidget;

namespace KGAPI2 {

/**
 * @headerfile authjob.h
 * @brief A job to authenticate against Google and fetch tokens
 *
 * This job can be either used to refresh expired tokens (this is usually done
 * automatically by Job implementation), or to request tokens for a new account.
 *
 * In the latter case, the AuthJob will automatically show a dialog where user
 * has to provide Google account credentials and grant access to all requested
 * scopes (@see Account::scopes).
 *
 * @author Daniel Vrátil <dvratil@redhat.com>
 * @since 2.0
 */
class KGAPICORE_EXPORT AuthJob : public KGAPI2::Job
{
    Q_OBJECT

  public:

    /**
     * @brief Creates a new authentication job that will use @p parent as parent
     *        for the authentication dialog.
     *
     * When constructed with a widget parent, AuthJob will place the
     * authentication widget on the @p parent instead of displaying a dialog.
     * This allows embedding the authentication process into a wizard for
     * instance.
     *
     * @param account Account to authenticate. When only scopes are set, a full
     *                authentication process will run (including showing the auth
     *                widget) and the rest will be filled by the job.
     *                @par
     *                Passing an Account with account name, scopes and both
     *                tokens filled will only refresh the access token. If however
     *                the scopes have been changed a full authentication will be
     *                started.
     *                @par
     *                Any other Account will be considered invalid and the job
     *                will finish immediately.
     *
     * @param apiKey Application API key
     * @param secretKey Application secret API key
     * @param parent Parent widget on which auth widget should be constructed if
     *               necessary.
     */
    explicit AuthJob(const AccountPtr &account, const QString &apiKey,
                     const QString &secretKey, QWidget* parent);

    /**
     * @brief Creates a new authentication job
     *
     * When constructed without a parent, or with a non-QWidget parent, the
     * job might pop up the authentication dialog.
     *
     * @param account Account to authenticate.
     *                See AuthJob(AccountPtr,QString,QString,QWidget) for 
     *                detailed description of @p account content.
     * @param apiKey Application API key
     * @param secretKey Application secret API key
     * @param parent
     */
    explicit AuthJob(const AccountPtr &account, const QString &apiKey,
                     const QString &secretKey, QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AuthJob() override;

    /**
     * @brief Returns reauthenticated account.
     *
     * @returns An account pointer passed to the AuthJob() constructor with
     *          all fields filled and validated. When the job fails, the account
     *          is unchanged.
     */
    AccountPtr account() const;

    /**
      * Sets the username that will be used when authenticate is called
      *
      * The username will be automatically filled in the Google login
      * form in the authentication widget.
      *
      * Be aware that the username will be set every time \sa authenticate is
      * called so if you want to change or remove it call \sa setUsername again
      * with empty string or \sa clearCredentials.
      *
      * @param username username to use
      */
    void setUsername(const QString &username);

    /**
     * Sets the password that will be used when authenticate is called
     *
     * The password will be automatically filled in the Google login
     * form in the authentication widget.
     *
     * Be aware that the password will be set every time \sa authenticate is
     * called so if you want to change or remove it call \sa setPassword again
     * with empty string or \sa clearCredentials.
     *
     * @param password password to use
     */
    void setPassword(const QString &password);

  protected:

    /**
     * @brief KGAPI2::Job::handleReply implementation
     *
     * @param reply
     * @param rawData
     */
    void handleReply(const QNetworkReply *reply, const QByteArray &rawData) override;

    /**
     * @brief KGAPI2::Job::displayRequest implementation
     *
     * @param accessManager
     * @param request
     * @param data
     * @param contentType
     */
    void dispatchRequest(QNetworkAccessManager* accessManager,
                                 const QNetworkRequest& request,
                                 const QByteArray& data,
                                 const QString& contentType) override;

    /**
     * @brief KGAPI2::Job::start implementation
     */
    void start() override;

  private:
    class Private;
    Private * const d;
    friend class Private;
};

} // namespace KGAPI2

#endif // LIBKGAPI2_AUTHJOB_H
