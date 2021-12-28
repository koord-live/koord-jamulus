/******************************************************************************\
 * Copyright (c) 2004-2020
 *
 * Author(s):
 *  Volker Fischer
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
\******************************************************************************/

#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWhatsThis>
#include <QTimer>
#include <QLocale>
#include <QtConcurrent>
#include "global.h"
#include "util.h"
#include "settings.h"
#include "multicolorled.h"
#include "ui_basicconnectdlgbase.h"

/* Definitions ****************************************************************/
// defines the time interval at which the request server list message is re-
// transmitted until it is received
#define SERV_LIST_REQ_UPDATE_TIME_MS 2000 // ms

/* Classes ********************************************************************/
class CBasicConnectDlg : public CBaseDlg, private Ui_CBasicConnectDlgBase
{
    Q_OBJECT

public:
    CBasicConnectDlg ( CClientSettings* pNSetP, QWidget* parent = nullptr );

    void SetConnClientsList ( const CHostAddress& InetAddr, const CVector<CChannelInfo>& vecChanInfo );

    QString GetSelectedAddress() const { return strSelectedAddress; }
    QString GetSelectedServerName() const { return strSelectedServerName; }

protected:

    CClientSettings* pSettings;

    QString      strSelectedAddress;
    QString      strSelectedServerName;

public slots:
    void OnConnectClicked();

};
