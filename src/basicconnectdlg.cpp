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

#include "basicconnectdlg.h"

/* Implementation *************************************************************/
CBasicConnectDlg::CBasicConnectDlg ( CClientSettings* pNSetP, QWidget* parent ) :
    CBaseDlg ( parent, Qt::Dialog ),
    pSettings ( pNSetP ),
    strSelectedAddress ( "" ),
    strSelectedServerName ( "" )

{
    setupUi ( this );

    // Add help text to controls -----------------------------------------------

    // init server address combo box (max MAX_NUM_SERVER_ADDR_ITEMS entries)
    cbxServerAddr->setMaxCount ( MAX_NUM_SERVER_ADDR_ITEMS );
    cbxServerAddr->setInsertPolicy ( QComboBox::NoInsert );

    // make sure the connect button has the focus
    butConnect->setFocus();

#ifdef ANDROID
    // for the android version maximize the window
    setWindowState ( Qt::WindowMaximized );
#endif

    // Connections -------------------------------------------------------------
    // combo boxes
//    QObject::connect ( cbxServerAddr, &QComboBox::editTextChanged, this, &CBasicConnectDlg::OnServerAddrEditTextChanged );

    // buttons
    QObject::connect ( butCancel, &QPushButton::clicked, this, &CBasicConnectDlg::close );

    QObject::connect ( butConnect, &QPushButton::clicked, this, &CBasicConnectDlg::OnConnectClicked );
}


void CBasicConnectDlg::OnConnectClicked()
{
    // get the IP address to be used according to the following definitions:
    // - if the list has focus and a line is selected, use this line
    // - if the list has no focus, use the current combo box text

    strSelectedAddress = NetworkUtil::FixAddress ( cbxServerAddr->currentText() );


    // tell the parent window that the connection shall be initiated
    done ( QDialog::Accepted );
}

