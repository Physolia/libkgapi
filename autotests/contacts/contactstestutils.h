/*
 * SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CONTACTSTESTUTILS_H_
#define CONTACTSTESTUTILS_H_

#include "types.h"
#include "contact.h"

KGAPI2::ContactPtr contactFromFile(const QString &path);
KGAPI2::ContactsGroupPtr contactsGroupFromFile(const QString &path);

#endif
