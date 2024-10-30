/******************************************************************************\
 * Copyright (c) 2004-2024
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

#include "clientdlg.h"
#include <QtConcurrent>
#include <QDesktopServices>
#include "urlhandler.h"
#include <kdapplication.h>

/* Implementation *************************************************************/
CClientDlg::CClientDlg ( CClient*         pNCliP,
                         CClientSettings* pNSetP,
                         const QString&   strConnOnStartupAddress,
                         const QString&   strMIDISetup,
                         const bool       bNewShowComplRegConnList,
                         const bool       bShowAnalyzerConsole,
                         const bool       bMuteStream,
                         const bool       bNEnableIPv6,
                         QWidget*         parent ) :
    QMainWindow (parent),
    pClient ( pNCliP ),
    pSettings ( pNSetP ),
    bConnectDlgWasShown ( false ),
    bDetectFeedback ( false ),
    bEnableIPv6 ( bNEnableIPv6 ),
    eLastRecorderState ( RS_UNDEFINED ), // for SetMixerBoardDeco
    eLastDesign ( GD_ORIGINAL ),         //          "
    strSelectedAddress (""),
    AnalyzerConsole ( pNCliP, parent )
{
    // setup main UI
    setupUi ( this );

    // set up net manager for https requests
    qNam = new QNetworkAccessManager;
    qNam->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

    // transitional settings view
    settingsView = new QQuickView();
    QQmlContext* settingsContext = settingsView->rootContext();
    settingsContext->setContextProperty("_settings", pNSetP );
    QWidget *settingsContainer = QWidget::createWindowContainer(settingsView, this);
    settingsView->setSource(QUrl("qrc:/settingview.qml"));
    settingsTab->layout()->addWidget(settingsContainer);

    // transitional main view
    mainView = new QQuickView();
    QQmlContext* mainContext = mainView->rootContext();
    mainContext->setContextProperty("_main", pNSetP );
    QWidget *mainContainer = QWidget::createWindowContainer(mainView, this);
    mainView->setSource(QUrl("qrc:/mainview.qml"));
    videoTab->layout()->addWidget(mainContainer);

    // initialize video_url with blank value to start
    strVideoUrl = "";

    // init GUI design
    // SetGUIDesign ( pClient->GetGUIDesign() );

    // MeterStyle init
    // SetMeterStyle ( pClient->GetMeterStyle() );
    // temp to show leds
    lbrInputLevelL->SetLevelMeterType ( CLevelMeter::MT_LED_STRIPE );
    lbrInputLevelR->SetLevelMeterType ( CLevelMeter::MT_LED_STRIPE );

    // set up settings
    // pSettings->setCbxAudioQuality(2); // high default
    emit pSettings->updateSettings();

    // set the settings pointer to the mixer board (must be done early)
    MainMixerBoard->SetSettingsPointer ( pSettings );
    MainMixerBoard->SetNumMixerPanelRows ( pSettings->iNumMixerPanelRows );

    // Pass through flag for MIDICtrlUsed
    MainMixerBoard->SetMIDICtrlUsed ( !strMIDISetup.isEmpty() );

    // reset mixer board
    MainMixerBoard->HideAll();

    // don't show mixerboard when no session
    MainMixerBoard->setMaximumHeight(0);

    // don't show Invite combobox
    inviteComboBox->setVisible(false);

    // don't show downloadUpdate button
    downloadLinkButton->setVisible(false);

    // init status label
    // OnTimerStatus();

    // init connection button text
    butConnect->setText ( tr ( "Join" ) );

    // init input level meter bars
    lbrInputLevelL->SetValue ( 0 );
    lbrInputLevelR->SetValue ( 0 );

    // init status LEDs
    ledBuffers->Reset();
    ledDelay->Reset();

    // init audio reverberation
    // sldAudioReverb->setRange ( 0, AUD_REVERB_MAX );
    // const int iCurAudReverb = pClient->GetReverbLevel();
    // sldAudioReverb->setValue ( iCurAudReverb );

    // init input boost
    pClient->SetInputBoost ( pSettings->iInputBoost );

    // init reverb channel
    // UpdateRevSelection();

    // set window title (with no clients connected -> "0")
    SetMyWindowTitle ( 0 );

    // track number of clients to detect joins/leaves for audio alerts
    iClients = 0;

    // prepare Mute Myself info label (invisible by default)
    // lblGlobalInfoLabel->setStyleSheet ( ".QLabel { background: red; }" );
    // lblGlobalInfoLabel->hide();

    // prepare update check info label (invisible by default)
    // lblUpdateCheck->setOpenExternalLinks ( true ); // enables opening a web browser if one clicks on a html link
    // lblUpdateCheck->setText ( "<font color=\"red\"><b>" + APP_UPGRADE_AVAILABLE_MSG_TEXT.arg ( APP_NAME ).arg ( VERSION ) + "</b></font>" );
    // lblUpdateCheck->hide();

    // setup timers
    TimerCheckAudioDeviceOk.setSingleShot ( true ); // only check once after connection
    TimerDetectFeedback.setSingleShot ( true );

    // Connect on startup ------------------------------------------------------
    if ( !strConnOnStartupAddress.isEmpty() )
    {
        // initiate connection (always show the address in the mixer board
        // (no alias))
        Connect ( strConnOnStartupAddress, strConnOnStartupAddress );
    }

    // Window positions --------------------------------------------------------
    // main window
    if ( !pSettings->vecWindowPosMain.isEmpty() && !pSettings->vecWindowPosMain.isNull() )
    {
        restoreGeometry ( pSettings->vecWindowPosMain );
    }

    // Connections -------------------------------------------------------------
    // push buttons
    QObject::connect ( butConnect, &QPushButton::clicked, this, &CClientDlg::OnConnectDisconBut );
    QObject::connect ( butNewStart, &QPushButton::clicked, this, &CClientDlg::OnNewStartClicked );
    // QObject::connect ( downloadLinkButton, &QPushButton::clicked, this, &CClientDlg::OnDownloadUpdateClicked );
    QObject::connect ( inviteComboBox, &QComboBox::activated, this, &CClientDlg::OnInviteBoxActivated );
    // QObject::connect ( checkUpdateButton, &QPushButton::clicked, this, &CClientDlg::OnCheckForUpdate );

    // connection for macOS custom url event handler
    // QObject::connect ( this, &CClientDlg::EventJoinConnectClicked, this, &CClientDlg::OnEventJoinConnectClicked );

    // check boxes
    // QObject::connect ( chbSettings, &QCheckBox::stateChanged, this, &CClientDlg::OnSettingsStateChanged );

//    QObject::connect ( chbPubJam, &QCheckBox::stateChanged, this, &CClientDlg::OnPubConnectStateChanged );

    // QObject::connect ( chbChat, &QCheckBox::stateChanged, this, &CClientDlg::OnChatStateChanged );

    QObject::connect ( chbLocalMute, &QCheckBox::stateChanged, this, &CClientDlg::OnLocalMuteStateChanged );

    // Connections -------------------------------------------------------------
    // list view
//    QObject::connect ( lvwServers, &QTreeWidget::itemDoubleClicked, this, &CClientDlg::OnServerListItemDoubleClicked );

    // to get default return key behaviour working
//    QObject::connect ( lvwServers, &QTreeWidget::activated, this, &CClientDlg::OnConnectClicked );

    // timers
    QObject::connect ( &TimerSigMet, &QTimer::timeout, this, &CClientDlg::OnTimerSigMet );

    QObject::connect ( &TimerBuffersLED, &QTimer::timeout, this, &CClientDlg::OnTimerBuffersLED );

    // QObject::connect ( &TimerStatus, &QTimer::timeout, this, &CClientDlg::OnTimerStatus );

    QObject::connect ( &TimerPing, &QTimer::timeout, this, &CClientDlg::OnTimerPing );

    // QObject::connect ( &TimerReRequestServList, &QTimer::timeout, this, &CClientDlg::OnTimerReRequestServList );

    QObject::connect ( &TimerCheckAudioDeviceOk, &QTimer::timeout, this, &CClientDlg::OnTimerCheckAudioDeviceOk );

    QObject::connect ( &TimerDetectFeedback, &QTimer::timeout, this, &CClientDlg::OnTimerDetectFeedback );

    // QObject::connect ( sldAudioReverb, &QDial::valueChanged, this, &CClientDlg::OnAudioReverbValueChanged );

    // radio buttons
    // QObject::connect ( rbtReverbSelL, &QRadioButton::clicked, this, &CClientDlg::OnReverbSelLClicked );

    // QObject::connect ( rbtReverbSelR, &QRadioButton::clicked, this, &CClientDlg::OnReverbSelRClicked );

    // other
    QObject::connect ( pClient, &CClient::ConClientListMesReceived, this, &CClientDlg::OnConClientListMesReceived );

    QObject::connect ( pClient, &CClient::Disconnected, this, &CClientDlg::OnDisconnected );

    QObject::connect ( pClient, &CClient::ClientIDReceived, this, &CClientDlg::OnClientIDReceived );

    QObject::connect ( pClient, &CClient::MuteStateHasChangedReceived, this, &CClientDlg::OnMuteStateHasChangedReceived );

    QObject::connect ( pClient, &CClient::RecorderStateReceived, this, &CClientDlg::OnRecorderStateReceived );

    QObject::connect ( pClient, &CClient::PingTimeReceived, this, &CClientDlg::OnPingTimeResult );

    // QObject::connect ( pClient, &CClient::CLServerListReceived, this, &CClientDlg::OnCLServerListReceived );

    // QObject::connect ( pClient, &CClient::CLRedServerListReceived, this, &CClientDlg::OnCLRedServerListReceived );

    // QObject::connect ( pClient, &CClient::CLConnClientsListMesReceived, this, &CClientDlg::OnCLConnClientsListMesReceived );

    // QObject::connect ( pClient, &CClient::CLPingTimeWithNumClientsReceived, this, &CClientDlg::OnCLPingTimeWithNumClientsReceived );

    QObject::connect ( pClient, &CClient::ControllerInFaderLevel, this, &CClientDlg::OnControllerInFaderLevel );

    QObject::connect ( pClient, &CClient::ControllerInPanValue, this, &CClientDlg::OnControllerInPanValue );

    QObject::connect ( pClient, &CClient::ControllerInFaderIsSolo, this, &CClientDlg::OnControllerInFaderIsSolo );

    QObject::connect ( pClient, &CClient::ControllerInFaderIsMute, this, &CClientDlg::OnControllerInFaderIsMute );

    QObject::connect ( pClient, &CClient::ControllerInMuteMyself, this, &CClientDlg::OnControllerInMuteMyself );

    QObject::connect ( pClient, &CClient::CLChannelLevelListReceived, this, &CClientDlg::OnCLChannelLevelListReceived );

    QObject::connect ( pClient, &CClient::VersionAndOSReceived, this, &CClientDlg::OnVersionAndOSReceived );

    QObject::connect ( pClient, &CClient::CLVersionAndOSReceived, this, &CClientDlg::OnCLVersionAndOSReceived );

    QObject::connect ( pClient, &CClient::SoundDeviceChanged, this, &CClientDlg::OnSoundDeviceChanged );

    // settings slots .....
    // QObject::connect ( this, &CClientSettingsDlg::GUIDesignChanged, this, &CClientDlg::OnGUIDesignChanged );

    // QObject::connect ( this, &CClientDlg::MeterStyleChanged, this, &CClientDlg::OnMeterStyleChanged );

    // QObject::connect ( this, &CClientDlg::AudioChannelsChanged, this, &CClientDlg::OnAudioChannelsChanged );

    // QObject::connect ( this, &CClientDlg::NumMixerPanelRowsChanged, this, &CClientDlg::OnNumMixerPanelRowsChanged );
    // end of settings slots --------------------------------

    QObject::connect ( MainMixerBoard, &CAudioMixerBoard::ChangeChanGain, this, &CClientDlg::OnChangeChanGain );

    QObject::connect ( MainMixerBoard, &CAudioMixerBoard::ChangeChanPan, this, &CClientDlg::OnChangeChanPan );

    QObject::connect ( MainMixerBoard, &CAudioMixerBoard::NumClientsChanged, this, &CClientDlg::OnNumClientsChanged );

    // QObject::connect ( this, &CClientDlg::ReqServerListQuery, this, &CClientDlg::OnReqServerListQuery );

    QObject::connect ( sessionCancelButton, &QPushButton::clicked, this, &CClientDlg::OnJoinCancelClicked );
    QObject::connect ( sessionConnectButton, &QPushButton::clicked, this, &CClientDlg::OnJoinConnectClicked );

    // note that this connection must be a queued connection, otherwise the server list ping
    // times are not accurate and the client list may not be retrieved for all servers listed
    // (it seems the sendto() function needs to be called from different threads to fire the
    // packet immediately and do not collect packets before transmitting)
    // QObject::connect ( this, &CClientDlg::CreateCLServerListPingMes, this, &CClientDlg::OnCreateCLServerListPingMes, Qt::QueuedConnection );

    // QObject::connect ( this, &CClientDlg::CreateCLServerListReqVerAndOSMes, this, &CClientDlg::OnCreateCLServerListReqVerAndOSMes );

    // QObject::connect ( this,
    //                    &CClientDlg::CreateCLServerListReqConnClientsListMes,
    //                    this,
    //                    &CClientDlg::OnCreateCLServerListReqConnClientsListMes );


    // Initializations which have to be done after the signals are connected ---
    // start timer for status bar
    TimerStatus.start ( LED_BAR_UPDATE_TIME_MS );

    sessionJoinWidget->setVisible(false);

    // mute stream on startup (must be done after the signal connections)
    if ( bMuteStream )
    {
        chbLocalMute->setCheckState ( Qt::Checked );
    }

    // // query the update server version number needed for update check (note
    // // that the connection less message respond may not make it back but that
    // // is not critical since the next time Jamulus is started we have another
    // // chance and the update check is not time-critical at all)
    // CHostAddress UpdateServerHostAddress;

    // do Koord version check - non-appstore versions only
    // check for existence of INSTALL_DIR/nonstore_donotdelete.txt

//     // check for existence of update-checker flagfile - to NOT use in stores (Apple in particular doesn't like)
// //    QString check_file_path = QApplication::applicationDirPath() +  "/nonstore_donotdelete.txt";
//     QFileInfo check_file(QApplication::applicationDirPath() +  "/nonstore_donotdelete.txt");
//     if (check_file.exists() && check_file.isFile()) {
//         OnCheckForUpdate();
//     }

    // // Send the request to two servers for redundancy if either or both of them
    // // has a higher release version number, the reply will trigger the notification.

    // if ( NetworkUtil().ParseNetworkAddress ( UPDATECHECK1_ADDRESS, UpdateServerHostAddress, bEnableIPv6 ) )
    // {
    //     pClient->CreateCLServerListReqVerAndOSMes ( UpdateServerHostAddress );
    // }

    // if ( NetworkUtil().ParseNetworkAddress ( UPDATECHECK2_ADDRESS, UpdateServerHostAddress, bEnableIPv6 ) )
    // {
    //     pClient->CreateCLServerListReqVerAndOSMes ( UpdateServerHostAddress );
    // }
}

void CClientDlg::closeEvent ( QCloseEvent* Event )
{
    // store window positions
    pSettings->vecWindowPosMain     = saveGeometry();
    AnalyzerConsole.close();

    // if connected, terminate connection
    if ( pClient->IsRunning() )
    {
        pClient->Stop();
    }

    // make sure all current fader settings are applied to the settings struct
    MainMixerBoard->StoreAllFaderSettings();

//    pSettings->bConnectDlgShowAllMusicians = ConnectDlg.GetShowAllMusicians();
    pSettings->eChannelSortType            = MainMixerBoard->GetFaderSorting();
    pSettings->iNumMixerPanelRows          = MainMixerBoard->GetNumMixerPanelRows();

    // default implementation of this event handler routine
    Event->accept();
}

void CClientDlg::OnJoinCancelClicked()
{
    HideJoinWidget();
}

void CClientDlg::OnEventJoinConnectClicked( const QString& url )
{
    strSelectedAddress = url;
    joinFieldEdit->setText ( url );
    OnJoinConnectClicked();
}

void CClientDlg::GetKoordAddress()
{
    QString err = endpoint_reply->errorString();
    int statusCode = endpoint_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString contents = QString::fromUtf8(endpoint_reply->readAll());
    qDebug() << "statuscode: " << statusCode;
    qDebug() << "endpoint reply: " << err;
    qDebug() << "contents of reply: " << contents;
    // response should be url style "koord://<host>:<port>" so assign this directly
    QString fixedAddress = NetworkUtil::FixAddress( contents.toUtf8() );
    qDebug() << "strSelectedAddress: " << fixedAddress;
    SetSelectedAddress( fixedAddress );
    CompleteConnection();
}

void CClientDlg::OnJoinConnectClicked()
{
    // process if user has entered [https://]koord.live/kd/?ks={hash} style url
    QRegularExpression rx("koord.live/kd/\\?ks=(.*)");
    QRegularExpressionMatch rx_match = rx.match(joinFieldEdit->text());
    if (rx_match.hasMatch()) {
        // We have http[s]::/koord.live/ style address, so look up session address from endpoint
        //        QString sessHash = rx_match.captured(1);
        // call https://koord.live/kd/?ks={hash}
        // MUST replace http with https to get SINGLE redirect from service
        QString https_url = joinFieldEdit->text().replace("http://", "https://");
        qInfo() << "Requesting koord url from : " << https_url;

        // ACTUALLY make the request
        endpoint_reply.reset(qNam->get(QNetworkRequest( https_url )));
        //FIXME - not sure why we need to connect here
        connect( endpoint_reply.get(), &QNetworkReply::finished, this, &CClientDlg::GetKoordAddress );
    } else {
        // clean up address
        strSelectedAddress = NetworkUtil::FixAddress ( joinFieldEdit->text() );
        CompleteConnection();
    }
}

void CClientDlg::CompleteConnection()
{
    // update joinFieldEdit with corrected address
    joinFieldEdit->setText(strSelectedAddress);

//    // tell the parent window that the connection shall be initiated
//    done ( QDialog::Accepted );
    //      ---    nabbed from OnConnectDlgAccepted():
    // only store new host address in our data base if the address is
    // not empty and it was not a server list item (only the addresses
    // typed in manually are stored by definition)
    if ( !strSelectedAddress.isEmpty() )
    {
        // store new address at the top of the list, if the list was already
        // full, the last element is thrown out
        pSettings->vstrIPAddress.StringFiFoWithCompare ( strSelectedAddress );
    }

    // get name to be set in audio mixer group box title
    QString strMixerBoardLabel;

    // an item of the server address combo box was chosen,
    // just show the address string as it was entered by the
    // user
    strMixerBoardLabel = strSelectedAddress;

    // first check if we are already connected, if this is the case we have to
    // disconnect the old server first
    if ( pClient->IsRunning() )
    {
        Disconnect();
    }

    // initiate connection
    Connect ( strSelectedAddress, strMixerBoardLabel );

    HideJoinWidget();
}

void CClientDlg::OnConnectDisconBut()
{
    // the connect/disconnect button implements a toggle functionality
    if ( pClient->IsRunning() )
    {
        Disconnect();
        // SetMixerBoardDeco ( RS_UNDEFINED, pClient->GetGUIDesign() );
    }
    else
    {
//        ShowBasicConnectionSetupDialog();
        ShowJoinWidget();
    }
}

void CClientDlg::OnInviteBoxActivated()
{
    QString text = inviteComboBox->currentText();

    QString subject = tr("Koord.Live - Session Invite");
    QString host_prefix = "";
    // if (devsetting1->text() != "")
    // {
    //     host_prefix = devsetting1->text() + ".";
    // }
    QString body = tr("You have an invite to play on Koord.Live.\n\n") +
                    tr("Click the Session Link to join your session.\n") +
                    tr("Session Link: https://%1koord.live/kd/?ks=%2 \n\n").arg(host_prefix, strSessionHash) +
                    tr("If you don't have the free Koord app installed yet,\n") +
                    tr("go to https://koord.live/downloads and follow the links.");

    if ( text.contains( "Copy Session Link" ) )
    {
        inviteComboBox->setCurrentIndex(0);
        QClipboard *clipboard = QGuiApplication::clipboard();
        // clipboard->setText(tr("https://%1koord.live/kd/?ks=%2 \n\n").arg(host_prefix, strSessionHash));
        // QToolTip::showText( inviteComboBox->mapToGlobal( QPoint( 0, 0 ) ), "Link Copied!" );
    }
    else if ( text.contains( "Share via Email" ) )
    {
        inviteComboBox->setCurrentIndex(0);
        QDesktopServices::openUrl(QUrl("mailto:?subject=" + subject + "&body=" + body, QUrl::TolerantMode));
    }
    else if ( text.contains( "Share via Whatsapp" ) )
    {
        inviteComboBox->setCurrentIndex(0);
        QDesktopServices::openUrl(QUrl("https://api.whatsapp.com/send?text=" + body, QUrl::TolerantMode));
    }
}

void CClientDlg::OnNewStartClicked()
{
    // open website session dashboard with region pre-selected
    // update current best region for start session param
    int idx = 0;
    // if ( lvwServers->topLevelItem ( 0 )->text ( 0 ).contains("Directory") )  {
    //     idx = 1;
    // }
    // strCurrBestRegion = lvwServers->topLevelItem ( idx )->text ( 0 )
    //                         .replace( matchState, "" )  // remove any state refs eg TX or DC
    //                         .replace( " ", "" )            // remove remaining whitespace eg in "New York"
    //                         .replace( ",", "" )            // remove commas, prob before
    //                         .replace( "Zürich", "Zurich" )      // special case #1
    //                         .replace( "SãoPaulo", "SaoPaulo" )  // special case #2
    //                         .toLower();
    // // qInfo() << strCurrBestRegion;
    QDesktopServices::openUrl(QUrl("https://koord.live/session?region=" + strCurrBestRegion, QUrl::TolerantMode));
}

void CClientDlg::OnClearAllStoredSoloMuteSettings()
{
    // if we are in an active connection, we first have to store all fader settings in
    // the settings struct, clear the solo and mute states and then apply the settings again
    MainMixerBoard->StoreAllFaderSettings();
    pSettings->vecStoredFaderIsSolo.Reset ( false );
    pSettings->vecStoredFaderIsMute.Reset ( false );
    MainMixerBoard->LoadAllFaderSettings();
}

void CClientDlg::OnLoadChannelSetup()
{
    QString strFileName = QFileDialog::getOpenFileName ( this, tr ( "Select Channel Setup File" ), "", QString ( "*." ) + MIX_SETTINGS_FILE_SUFFIX );

    if ( !strFileName.isEmpty() )
    {
        // first update the settings struct and then update the mixer panel
        pSettings->LoadFaderSettings ( strFileName );
        MainMixerBoard->LoadAllFaderSettings();
    }
}

void CClientDlg::OnSaveChannelSetup()
{
    QString strFileName = QFileDialog::getSaveFileName ( this, tr ( "Select Channel Setup File" ), "", QString ( "*." ) + MIX_SETTINGS_FILE_SUFFIX );

    if ( !strFileName.isEmpty() )
    {
        // first store all current fader settings (in case we are in an active connection
        // right now) and then save the information in the settings struct in the file
        MainMixerBoard->StoreAllFaderSettings();
        pSettings->SaveFaderSettings ( strFileName );
    }
}

void CClientDlg::OnVersionAndOSReceived ( COSUtil::EOpSystemType, QString strVersion )
{
    // check if Pan is supported by the server (minimum version is 3.5.4)
#if QT_VERSION >= QT_VERSION_CHECK( 5, 6, 0 )
    if ( QVersionNumber::compare ( QVersionNumber::fromString ( strVersion ), QVersionNumber ( 3, 5, 4 ) ) >= 0 )
    {
        MainMixerBoard->SetPanIsSupported();
    }
#endif
}

void CClientDlg::OnCLVersionAndOSReceived ( CHostAddress, COSUtil::EOpSystemType, QString strVersion )
{
    // update check
#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 6, 0 ) ) && !defined( DISABLE_VERSION_CHECK )
    int            mySuffixIndex;
    QVersionNumber myVersion = QVersionNumber::fromString ( VERSION, &mySuffixIndex );

    int            serverSuffixIndex;
    QVersionNumber serverVersion = QVersionNumber::fromString ( strVersion, &serverSuffixIndex );

    // only compare if the server version has no suffix (such as dev or beta)
    if ( strVersion.size() == serverSuffixIndex && QVersionNumber::compare ( serverVersion, myVersion ) > 0 )
    {
        // do nothing
        ;
        // show the label and hide it after one minute again
//        lblUpdateCheck->show();
//        QTimer::singleShot ( 60000, [this]() { lblUpdateCheck->hide(); } );
    }
#endif
}

void CClientDlg::OnConClientListMesReceived ( CVector<CChannelInfo> vecChanInfo )
{
    // update mixer board with the additional client infos
    MainMixerBoard->ApplyNewConClientList ( vecChanInfo );
    // set session status bar with address
    sessionStatusLabel->setText("CONNECTED");
    sessionStatusLabel->setStyleSheet ( "QLabel { color: green; font: bold; }" );

}

void CClientDlg::OnNumClientsChanged ( int iNewNumClients )
{
    if ( pSettings->bEnableAudioAlerts && iNewNumClients > iClients )
    {
//        QSoundEffect* sf = new QSoundEffect();
//        sf->setSource ( QUrl::fromLocalFile ( ":sounds/res/sounds/new_user.wav" ) );
//        sf->play();
        ; // do nothing for now
    }

    // iNewNumClients will be zero on the first trigger of this signal handler when connecting to a new server.
    // Subsequent triggers will thus sound the alert (if enabled).
    iClients = iNewNumClients;

    // update window title
    SetMyWindowTitle ( iNewNumClients );
}

void CClientDlg::SetMyWindowTitle ( const int iNumClients )
{
    // set the window title (and therefore also the task bar icon text of the OS)
    // according to the following specification (#559):
    // <ServerName> - <N> users - Jamulus
    QString    strWinTitle;
    const bool bClientNameIsUsed = !pClient->strClientName.isEmpty();

    if ( bClientNameIsUsed )
    {
        // if --clientname is used, the APP_NAME must be the very first word in
        // the title, otherwise some user scripts do not work anymore, see #789
        strWinTitle += QString ( APP_NAME ) + " - " + pClient->strClientName + " ";
    }

    if ( iNumClients == 0 )
    {
        // only application name
        if ( !bClientNameIsUsed ) // if --clientname is used, the APP_NAME is the first word in title
        {
            strWinTitle += QString ( APP_NAME );
        }
    }
    else
    {
        strWinTitle += MainMixerBoard->GetServerName();

        if ( iNumClients == 1 )
        {
            strWinTitle += " - 1 " + tr ( "user" );
        }
        else if ( iNumClients > 1 )
        {
            strWinTitle += " - " + QString::number ( iNumClients ) + " " + tr ( "users" );
        }

        if ( !bClientNameIsUsed ) // if --clientname is used, the APP_NAME is the first word in title
        {
            strWinTitle += " - " + QString ( APP_NAME );
        }
    }

    setWindowTitle ( strWinTitle );

#if defined( Q_OS_MACOS )
    // for MacOS only we show the number of connected clients as a
    // badge label text if more than one user is connected
    if ( iNumClients > 1 )
    {
        // show the number of connected clients
        SetMacBadgeLabelText ( QString ( "%1" ).arg ( iNumClients ) );
    }
    else
    {
        // clear the text (apply an empty string)
        SetMacBadgeLabelText ( "" );
    }
#endif
}

void CClientDlg::ShowJoinWidget()
{
    sessionJoinWidget->setVisible(true);
    defaultButtonWidget->setVisible(false);
}

void CClientDlg::HideJoinWidget()
{
    sessionJoinWidget->setVisible(false);
    defaultButtonWidget->setVisible(true);
}

void CClientDlg::ShowAnalyzerConsole()
{
    // open analyzer console dialog
    AnalyzerConsole.show();

    // make sure dialog is upfront and has focus
    AnalyzerConsole.raise();
    AnalyzerConsole.activateWindow();
}

void CClientDlg::OnLocalMuteStateChanged ( int value )
{
    pClient->SetMuteOutStream ( value == Qt::Checked );

    // show/hide info label
    if ( value == Qt::Checked )
    {
        // do nothing
        ;
//        lblGlobalInfoLabel->show();
    }
    else
    {
        // do nothing
        ;
//        lblGlobalInfoLabel->hide();
    }
}

void CClientDlg::OnTimerSigMet()
{
    // show current level
    lbrInputLevelL->SetValue ( pClient->GetLevelForMeterdBLeft() );
    lbrInputLevelR->SetValue ( pClient->GetLevelForMeterdBRight() );

    if ( bDetectFeedback &&
         ( pClient->GetLevelForMeterdBLeft() > NUM_STEPS_LED_BAR - 0.5 || pClient->GetLevelForMeterdBRight() > NUM_STEPS_LED_BAR - 0.5 ) )
    {
        // mute locally and mute channel
        chbLocalMute->setCheckState ( Qt::Checked );
        MainMixerBoard->MuteMyChannel();

        // show message box about feedback issue
        QCheckBox* chb = new QCheckBox ( tr ( "Enable feedback detection" ) );
        chb->setCheckState ( pSettings->bEnableFeedbackDetection ? Qt::Checked : Qt::Unchecked );
        QMessageBox msgbox;
        msgbox.setText ( tr ( "Audio feedback or loud signal detected.\n\n"
                              "We muted your channel and activated 'Mute Myself'. Please solve "
                              "the feedback issue first and unmute yourself afterwards." ) );
        msgbox.setIcon ( QMessageBox::Icon::Warning );
        msgbox.addButton ( QMessageBox::Ok );
        msgbox.setDefaultButton ( QMessageBox::Ok );
        msgbox.setCheckBox ( chb );

        // QObject::connect ( chb, &QCheckBox::stateChanged, this, &CClientDlg::OnFeedbackDetectionChanged );

        msgbox.exec();
    }
}

void CClientDlg::OnTimerBuffersLED()
{
    CMultiColorLED::ELightColor eCurStatus;

    // get and reset current buffer state and set LED from that flag
    if ( pClient->GetAndResetbJitterBufferOKFlag() )
    {
        eCurStatus = CMultiColorLED::RL_GREEN;
    }
    else
    {
        eCurStatus = CMultiColorLED::RL_RED;
    }

    // update the buffer LED and the general settings dialog, too
    ledBuffers->SetLight ( eCurStatus );
}

void CClientDlg::OnTimerPing()
{
    // send ping message to the server
    pClient->CreateCLPingMes();
}

void CClientDlg::OnPingTimeResult ( int iPingTime )
{
    // calculate overall delay
    const int iOverallDelayMs = pClient->EstimatedOverallDelay ( iPingTime );

    // color definition: <= 43 ms green, <= 68 ms yellow, otherwise red
    CMultiColorLED::ELightColor eOverallDelayLEDColor;

    if ( iOverallDelayMs <= 43 )
    {
        eOverallDelayLEDColor = CMultiColorLED::RL_GREEN;
    }
    else
    {
        if ( iOverallDelayMs <= 68 )
        {
            eOverallDelayLEDColor = CMultiColorLED::RL_YELLOW;
        }
        else
        {
            eOverallDelayLEDColor = CMultiColorLED::RL_RED;
        }
    }

    // only update delay information on settings dialog if it is visible to
    // avoid CPU load on working thread if not necessary
    // FIXME - change to if settings TAB is visible

    if ( settingsTab->isVisible())
        // UpdateUploadRate(); // set ping time result to settings tab
        //FIXME: emit uploadRateChanged
        // emit CClientSettings::uploadRateChanged();

    SetPingTime ( iPingTime, iOverallDelayMs, eOverallDelayLEDColor );

    // update delay LED on the main window
    ledDelay->SetLight ( eOverallDelayLEDColor );
}

void CClientDlg::OnTimerCheckAudioDeviceOk()
{
    // check if the audio device entered the audio callback after a pre-defined
    // timeout to check if a valid device is selected and if we do not have
    // fundamental settings errors (in which case the GUI would only show that
    // it is trying to connect the server which does not help to solve the problem (#129))
    if ( !pClient->IsCallbackEntered() )
    {
        QMessageBox::warning ( this,
                               APP_NAME,
                               tr ( "Your sound card is not working correctly. "
                                    "Please open the settings dialog and check the device selection and the driver settings." ) );
    }
}

void CClientDlg::OnTimerDetectFeedback() { bDetectFeedback = false; }

void CClientDlg::OnSoundDeviceChanged ( QString strError )
{
    if ( !strError.isEmpty() )
    {
        // the sound device setup has a problem, disconnect any active connection
        if ( pClient->IsRunning() )
        {
            Disconnect();
        }

        // show the error message of the device setup
        QMessageBox::critical ( this, APP_NAME, strError, tr ( "Ok" ), nullptr );
    }

    // if the check audio device timer is running, it must be restarted on a device change
    if ( TimerCheckAudioDeviceOk.isActive() )
    {
        TimerCheckAudioDeviceOk.start ( CHECK_AUDIO_DEV_OK_TIME_MS );
    }

    if ( pSettings->bEnableFeedbackDetection && TimerDetectFeedback.isActive() )
    {
        TimerDetectFeedback.start ( DETECT_FEEDBACK_TIME_MS );
        bDetectFeedback = true;
    }

    // update the settings
    // UpdateSoundDeviceChannelSelectionFrame();
// #if defined( Q_OS_WINDOWS )
//     SetupBuiltinASIOBox();
// #endif
}

void CClientDlg::Connect ( const QString& strSelectedAddress, const QString& strMixerBoardLabel )
{
    // set address and check if address is valid
    if ( pClient->SetServerAddr ( strSelectedAddress ) )
    {
        // try to start client, if error occurred, do not go in
        // running state but show error message
        try
        {
            if ( !pClient->IsRunning() )
            {
                pClient->Start();
            }
        }

        catch ( const CGenErr& generr )
        {
            // show error message and return the function
            QMessageBox::critical ( this, APP_NAME, generr.GetErrorText(), "Close", nullptr );
            return;
        }

        // set session status bar
//        sessionStatusLabel->setText(strSelectedAddress);
//        sessionStatusLabel->setText("CONNECTED");
//        sessionStatusLabel->setStyleSheet ( "QLabel { color: green; font: bold; }" );

        // hide label connect to server
//        lblConnectToServer->hide();
        lbrInputLevelL->setEnabled ( true );
        lbrInputLevelR->setEnabled ( true );

        // change connect button text to "disconnect"
        butConnect->setText ( tr ( "&Disconnect" ) );
        butConnect->setToolTip( tr("Click to leave the Session"));
        QString qss = QString("background-color: red");
        butConnect->setStyleSheet(qss);
        inviteComboBox->setVisible(true);
        QListView *view = new QListView(inviteComboBox);
        view->setStyleSheet("QListView::item{height: 50px}");
        inviteComboBox->setView(view);
        inviteComboBox->addItem("Invite ...");
        inviteComboBox->addItem(QIcon(":/svg/main/res/copy-link.svg"), "Copy Session Link");
        inviteComboBox->addItem(QIcon(":/svg/main/res/mail-to.svg"), "Share via Email");
        inviteComboBox->addItem(QIcon(":/svg/main/res/whatsapp.svg"), "Share via Whatsapp");
        butNewStart->setVisible(false);
        defaultButtonWidget->setMaximumHeight(30);
        // hide regionchecker
        regionChecker->setVisible(false);

        TimerReRequestServList.stop();
        //FIXME - UI hacks for mixerboard space
        verticalSpacerGroupTop->changeSize(20,0);
        verticalSpacerGroupMid->changeSize(20,0);

        //FIXME - workaround for error window on macOS
        // switch to video tab immediately to prevent webview from NOT loading
//#if defined( Q_OS_MACOS )
//        tabWidget->setCurrentIndex(1);
//#endif

//        // enable chat widgets
//        butSend->setEnabled(true);
//        edtLocalInputText->setEnabled(true);

        // set connection status in status bar
        if (strMixerBoardLabel.isEmpty())
        {
            sessionStatusLabel->setText("NO SESSION");
            sessionStatusLabel->setStyleSheet ( "QLabel { color: gray; font: bold; }" );
        }
        else
        {
            sessionStatusLabel->setText("CONNECTING...");
            sessionStatusLabel->setStyleSheet ( "QLabel { color: orange; font: bold; }" );
        }
        // session is live, show mixerboard
        MainMixerBoard->setMaximumHeight(2000);
//        MainMixerBoard->SetServerName ( strMixerBoardLabel );

        // start timer for level meter bar and ping time measurement
        TimerSigMet.start ( LEVELMETER_UPDATE_TIME_MS );
        TimerBuffersLED.start ( BUFFER_LED_UPDATE_TIME_MS );
        TimerPing.start ( PING_UPDATE_TIME_MS );
        TimerCheckAudioDeviceOk.start ( CHECK_AUDIO_DEV_OK_TIME_MS ); // is single shot timer

        // audio feedback detection
        if ( pSettings->bEnableFeedbackDetection )
        {
            TimerDetectFeedback.start ( DETECT_FEEDBACK_TIME_MS ); // single shot timer
            bDetectFeedback = true;
        }

        // do video_url lookup here ...
        QUrl url("https://koord.live/sess/sessionvideourl/");
        //FIXME - for test only
        // if (devsetting1->text() != "")
        // {
        //     url.setUrl(QString("https://%1.koord.live/sess/sessionvideourl/").arg(devsetting1->text()));
        // }
        QNetworkRequest request(url);
        //FIXME - for test only
//        if (devsetting1->text() != "")
//        {
//            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//            request.setRawHeader(devsetting2->text().toUtf8(), devsetting3->text().toUtf8());
//        }

        QRegularExpression rx_sessaddr("^(([a-z]*[0-9]*\\.*)+):([0-9]+)$");
        QRegularExpressionMatch reg_match = rx_sessaddr.match(strSelectedAddress);
        QString hostname;
        QString port;
        if (reg_match.hasMatch()) {
            hostname = reg_match.captured(1);
            port = reg_match.captured(3);
        }

        // create request body
        // const QByteArray vid_req = "{'audio_port': '23455', 'session_dns': 'lively.kv.koord.live'}";
        QJsonObject obj;
        obj["audio_port"] = port;
        obj["session_dns"] = hostname;
        QJsonDocument doc(obj);
        QByteArray vid_req = doc.toJson();

        // send request and assign reply pointer
        QNetworkReply *reply = qNam->post(request, vid_req);

        // connect reply pointer with callback for finished signal
        QObject::connect(reply, &QNetworkReply::finished, this, [=]()
            {
                QString err = reply->errorString();
                QString contents = QString::fromUtf8(reply->readAll());
                // if reply - no error
                // parse the JSON response
                QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
                QJsonObject jsonObject = jsonResponse.object();
                strVideoHost = jsonObject.value("video_url").toString();
                strSessionHash = jsonObject.value("session_hash").toString();

                // set the video url and update QML side

                // redirect causes empty return vals first time round - wait until values are non-empty
                if ( !strVideoHost.isEmpty() )
                {
                    strVideoUrl = strVideoHost + "#userInfo.displayName=\"" + pClient->ChannelInfo.strName + "\"";
                    qInfo() << "strVideoHost: " << strVideoHost << ", strVideoUrl: " << strVideoUrl;
                    // tell the QML side that value is updated
                    emit videoUrlChanged();
                }
            });
    }
}

void CClientDlg::Disconnect()
{
    // only stop client if currently running, in case we received
    // the stopped message, the client is already stopped but the
    // connect/disconnect button and other GUI controls must be
    // updated
    if ( pClient->IsRunning() )
    {
        pClient->Stop();
    }

    // change connect button text to "connect"
    defaultButtonWidget->setMaximumHeight(80);
    butConnect->setStyleSheet(QString("background-color: rgb(251, 143, 0);"));
    butConnect->setToolTip("Click to Join a Session that's already started");
//    linkField->setVisible(false);
    butNewStart->setVisible(true);
    inviteComboBox->setVisible(false);
    inviteComboBox->clear();
    butConnect->setText ( tr ( "Join" ) );
    // show RegionChecker again
    regionChecker->setVisible(true);
    TimerReRequestServList.start();
    //FIXME - UI hacks to make space for mixerboard
    verticalSpacerGroupTop->changeSize(20,40);
    verticalSpacerGroupMid->changeSize(20,40);

//    // disable chat widgets
//    butSend->setEnabled(false);
//    edtLocalInputText->setEnabled(false);

    // reset Rec label (if server ends session)
    recLabel->setStyleSheet ( "QLabel { color: rgb(86, 86, 86); font: normal; }" );

    // reset session status bar
    sessionStatusLabel->setText("NO SESSION");
    sessionStatusLabel->setStyleSheet ( "QLabel { color: white; font: normal; }" );
    // reset server name in audio mixer group box title
//    MainMixerBoard->SetServerName ( "" );

    // Reset video view to No Session
//FIXME - we do special stuff for Android, because calling setSource() again causes webview to go FULL SCREEN
// #if defined(Q_OS_ANDROID)
//     strVideoUrl = "https://koord.live/about";
//     emit videoUrlChanged();
//     strVideoUrl = "";
//     emit videoUrlChanged();
// #else
//     videoView->setSource(QUrl("qrc:/nosessionview.qml"));
// #endif

    // stop timer for level meter bars and reset them
    TimerSigMet.stop();
    lbrInputLevelL->setEnabled ( false );
    lbrInputLevelR->setEnabled ( false );
    lbrInputLevelL->SetValue ( 0 );
    lbrInputLevelR->SetValue ( 0 );

    // show connect to server message
//    lblConnectToServer->show();

    // stop other timers
    TimerBuffersLED.stop();
    TimerPing.stop();
    TimerCheckAudioDeviceOk.stop();
    TimerDetectFeedback.stop();
    bDetectFeedback = false;

    //### TODO: BEGIN ###//
    // is this still required???
    // immediately update status bar
    // OnTimerStatus();
    //### TODO: END ###//

    // reset LEDs
    ledBuffers->Reset();
    ledDelay->Reset();

    // clear text labels with client parameters
    lblPingVal->setText ( "---" );
    lblPingUnit->setText ( "" );
    lblDelayVal->setText ( "---" );
    lblDelayUnit->setText ( "" );

    // clear mixer board (remove all faders)
    MainMixerBoard->HideAll();
    MainMixerBoard->setMaximumHeight(0);
}

void CClientDlg::replyFinished(QNetworkReply *rep)
{
    QByteArray bts = rep->readAll();
    QString str(bts);
    QMessageBox::information(this,"sal",str,"ok");
}

void CClientDlg::OnRecorderStateReceived ( const ERecorderState newRecorderState )
{
    MainMixerBoard->SetRecorderState ( newRecorderState );
    // SetMixerBoardDeco ( newRecorderState, pClient->GetGUIDesign() );
}

void CClientDlg::SetPingTime ( const int iPingTime, const int iOverallDelayMs, const CMultiColorLED::ELightColor eOverallDelayLEDColor )
{
    // apply values to GUI labels, take special care if ping time exceeds
    // a certain value
    if ( iPingTime > 500 )
    {
        const QString sErrorText = "<font color=\"red\"><b>&#62;500</b></font>";
        lblPingVal->setText ( sErrorText );
        lblDelayVal->setText ( sErrorText );
    }
    else
    {
        lblPingVal->setText ( QString().setNum ( iPingTime ) );
        lblDelayVal->setText ( QString().setNum ( iOverallDelayMs ) );
    }
    lblPingUnit->setText ( "ms" );
    lblDelayUnit->setText ( "ms" );

    // set current LED status
    ledDelay->SetLight ( eOverallDelayLEDColor );
}

void CClientDlg::OnConnectFromURLHandler(const QString& connect_url)
{
    // connect directly to url koord://fqdnfqdn.kv.koord.live:30231
    qInfo() << "OnConnectFromURLHandler URL: " << connect_url;
//    QString connect_addr = connect_url.replace("koord://", "");
    qInfo() << "OnConnectFromURLHandler connect_addr: " << connect_url;
    strSelectedAddress = connect_url;
    // set text in the dialog as well to keep OnJoinConnectClicked() implementation consistent
    joinFieldEdit->setText(strSelectedAddress);
    emit EventJoinConnectClicked( connect_url );
}
