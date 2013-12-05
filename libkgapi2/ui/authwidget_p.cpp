/*
    Copyright 2012, 2013  Daniel Vrátil <dvratil@redhat.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "authwidget_p.h"
#include "account.h"
#include "debug.h"
#include "accountinfo/accountinfo.h"
#include "accountinfo/accountinfofetchjob.h"
#include "private/newtokensfetchjob_p.h"

#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebElement>

#include <qjson/parser.h>

#include <KDE/KUrl>
#include <KDE/KLocalizedString>
#include <KIO/AccessManager>
#include <KDE/KWebView>

#include <QtCore/QStringBuilder>


using namespace KGAPI2;


AuthWidget::Private::Private(AuthWidget *parent):
    QObject(),
    showProgressBar(true),
    progress(AuthWidget::None),
    q(parent)
{
    setupUi();
}

AuthWidget::Private::~Private()
{
}

void AuthWidget::Private::setupUi()
{
    vbox = new QVBoxLayout(q);
    q->setLayout(vbox);

    label = new QLabel(q);
    label->setText(QLatin1String("<b>") % i18n("Authorizing token. This should take just a moment...") % QLatin1String("</b>"));
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    label->setVisible(false);
    vbox->addWidget(label);

    progressbar = new QProgressBar(q);
    progressbar->setMinimum(0);
    progressbar->setMaximum(100);
    progressbar->setValue(0);
    vbox->addWidget(progressbar);

    webview = new KWebView(q);
    vbox->addWidget(webview);
    connect(webview, SIGNAL(loadProgress(int)), progressbar, SLOT(setValue(int)));
    connect(webview, SIGNAL(urlChanged(QUrl)), this, SLOT(webviewUrlChanged(QUrl)));
    connect(webview, SIGNAL(loadFinished(bool)), this, SLOT(webviewFinished()));
}

void AuthWidget::Private::setProgress(AuthWidget::Progress progress)
{
    KGAPIDebug() << progress;
    this->progress = progress;
    Q_EMIT q->progress(progress);
}


void AuthWidget::Private::emitError(const enum Error errCode, const QString& msg)
{
    label->setVisible(true);
    webview->setVisible(false);
    progressbar->setVisible(false);

    label->setText(QLatin1String("<b>") % msg % QLatin1String("</b>"));

    Q_EMIT q->error(errCode, msg);
    setProgress(AuthWidget::Error);
}


void AuthWidget::Private::webviewUrlChanged(const QUrl &url)
{
    KGAPIDebug() << url;

    /* Access token here - hide browser and tell user to wait until we
     * finish the authentication process ourselves */
    if (url.host() == QLatin1String("accounts.google.com") && url.path() == QLatin1String("/o/oauth2/approval")) {
        webview->setVisible(false);
        progressbar->setVisible(false);
        label->setVisible(true);

        setProgress(AuthWidget::TokensRetrieval);
    }
}

void AuthWidget::Private::webviewFinished()
{
    QUrl url = webview->url();
    KGAPIDebug() << url;

    if (url.host() == QLatin1String("accounts.google.com") && url.path() == QLatin1String("/ServiceLogin")) {
        if (username.isEmpty() && password.isEmpty()) {
            return;
        }

        QWebFrame *frame = webview->page()->mainFrame();
        if (!username.isEmpty()) {
            QWebElement email = frame->findFirstElement(QLatin1String("input#Email"));
            if (!email.isNull()) {
                email.setAttribute(QLatin1String("value"), username);
            }
        }

        if (!password.isEmpty()) {
            QWebElement passd = frame->findFirstElement(QLatin1String("input#Passwd"));
            if (!passd.isNull()) {
                passd.setAttribute(QLatin1String("value"), password);
            }
        }

        return;
    }

    if (url.host() == QLatin1String("accounts.google.com") && url.path() == QLatin1String("/o/oauth2/approval")) {
        QString title = webview->title();
        QString token;

        if (title.startsWith(QLatin1String("success"), Qt::CaseInsensitive)) {
            int pos = title.indexOf(QLatin1String("code="));
            /* Skip the 'code=' string as well */
            token = title.mid (pos + 5);
        } else {
            KGAPIDebug() << "Parsing token page failed. Title:" << title;
            KGAPIDebugRawData() << webview->page()->mainFrame()->toHtml();
            emitError(AuthError, i18n("Parsing token page failed."));
            return;
        }

        if (token.isEmpty()) {
            KGAPIDebug() << "Failed to obtain token.";
            KGAPIDebugRawData() << webview->page()->mainFrame()->toHtml();
            emitError(AuthError, i18n("Failed to obtain token."));
            return;
        }

        KGAPI2::NewTokensFetchJob *fetchJob = new KGAPI2::NewTokensFetchJob(token, apiKey, secretKey);
        connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)),
                SLOT(tokensReceived(KGAPI2::Job*)));
    }
}

void AuthWidget::Private::tokensReceived(KGAPI2::Job* job)
{
    KGAPI2::NewTokensFetchJob *tokensFetchJob = qobject_cast<KGAPI2::NewTokensFetchJob*>(job);

    account->setAccessToken(tokensFetchJob->accessToken());
    account->setRefreshToken(tokensFetchJob->refreshToken());
    tokensFetchJob->deleteLater();

    KGAPI2::AccountInfoFetchJob *fetchJob = new KGAPI2::AccountInfoFetchJob(account, this);

    connect(fetchJob, SIGNAL(finished(KGAPI2::Job*)),
            this, SLOT(accountInfoReceived(KGAPI2::Job*)));
    KGAPIDebug() << "Requesting AccountInfo";
}

void AuthWidget::Private::accountInfoReceived(KGAPI2::Job* job)
{
    if (job->error()) {
        KGAPIDebug() << "Error when retrieving AccountInfo:" << job->errorString();
        emitError((enum Error) job->error(), job->errorString());
        return;
    }

    KGAPI2::ObjectsList objects = qobject_cast<KGAPI2::AccountInfoFetchJob*>(job)->items();
    Q_ASSERT(!objects.isEmpty());

    KGAPI2::AccountInfoPtr accountInfo = objects.first().staticCast<KGAPI2::AccountInfo>();
    account->setAccountName(accountInfo->email());

    job->deleteLater();

    Q_EMIT q->authenticated(account);
    setProgress(AuthWidget::Finished);
}

