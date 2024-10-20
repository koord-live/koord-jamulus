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
// #include <QtQuickWidgets>
//#include "unsafearea.h"
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
    //FIXME cruft to remove later - just remove compiler warnings for now
    // if (bNewShowComplRegConnList == false)
    //     ;
    // if (bShowAnalyzerConsole == false)
    //     ;
    // end cruft

    //FIXME - possibly not necessary
#if defined(Q_OS_ANDROID)
    setCentralWidget(backgroundFrame);
#endif

    // setup main UI
    setupUi ( this );

//    // if on iPhone / iPad (notches prob only on iPhone)
//#if defined(Q_OS_IOS)
//    QSize size = qApp->screens()[0]->size();
//    int _height = size.height();
//    int _width = size.width();
//    QScreen* screen = QGuiApplication::primaryScreen();
//    int _dpi = screen->devicePixelRatio();
//    mUnsafeArea->configureDevice(_height, _width, _dpi);
//    mUnsafeArea->orientationChanged(2); // force landscape left for now
//    int safeWidth = _width - mUnsafeArea->unsafeLeftMargin()- mUnsafeArea->unsafeRightMargin();
////            - (isTabletInLandscape? drawerWidth : 0)
//    int safeHeight = _height - mUnsafeArea->unsafeTopMargin() - mUnsafeArea->unsafeBottomMargin();
//    this->setFixedSize(QSize(safeWidth, safeHeight));
//#endif

    // set up net manager for https requests
    qNam = new QNetworkAccessManager;
    qNam->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

// #if defined( Q_OS_WINDOWS )
//     kdasio_setup();
// #endif

    // regionChecker stuff
//    // setup dir servers
//    // set up list view for connected clients (note that the last column size
//    // must not be specified since this column takes all the remaining space)
//#if defined(Q_OS_ANDROID)
//    // for Android we need larger numbers because of the default font size
//    lvwServers->setColumnWidth ( 0, 200 );
//    lvwServers->setColumnWidth ( 1, 130 );
//    lvwServers->setColumnWidth ( 2, 100 );
//#else
    lvwServers->setColumnWidth ( 0, 150 );
    lvwServers->setColumnWidth ( 1, 100 );
    lvwServers->setColumnWidth ( 2, 50 );
//    lvwServers->setColumnWidth ( 3, 220 );
//#endif
    lvwServers->clear();

//    // make sure we do not get a too long horizontal scroll bar
//    lvwServers->header()->setStretchLastSection ( false );

//    // add invisible columns which are used for sorting the list and storing
//    // the current/maximum number of clients
//    // 0: server name
//    // 1: ping time
//    // 2: number of musicians (including additional strings like " (full)")
//    // 3: location
//    // 4: minimum ping time (invisible)
//    // 5: maximum number of clients (invisible)
//    lvwServers->setColumnCount ( 3 );
//    lvwServers->hideColumn ( 4 );
//    lvwServers->hideColumn ( 5 );

//    // per default the root shall not be decorated (to save space)
    lvwServers->setRootIsDecorated ( false );

//    // setup timers
    TimerInitialSort.setSingleShot ( true ); // only once after list request

// // FIXME - exception for Android
// // - QuickView does NOT work as for other OS - when setSource is changed from "nosession" to webview, view goes full-screen
// #if defined(Q_OS_ANDROID)
//     quickWidget = new QQuickWidget;
//     // need SizeRootObjectToView for QuickWidget, otherwise web content doesn't load ??
//     quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
//     quickWidget->setSource(QUrl("qrc:/androidwebview.qml"));
//     videoTab->layout()->addWidget(quickWidget);
//     QQmlContext* context = quickWidget->rootContext();
// #else
// // Note: use QuickView to workaround problems with QuickWidget
//     videoView = new QQuickView();
//     QWidget *container = QWidget::createWindowContainer(videoView, this);
//     videoView->setSource(QUrl("qrc:/nosessionview.qml"));
//     videoTab->layout()->addWidget(container);
//     QQmlContext* context = videoView->rootContext();
// // #endif
//     context->setContextProperty("_clientdlg", this);

    // transitional settings view
    settingsView = new QQuickView();
    QQmlContext* settingsContext = settingsView->rootContext();
    settingsContext->setContextProperty("_settings", pNSetP );
    QWidget *settingsContainer = QWidget::createWindowContainer(settingsView, this);
    settingsView->setSource(QUrl("qrc:/settingview.qml"));
    settingsTab->layout()->addWidget(settingsContainer);

    // initialize video_url with blank value to start
    strVideoUrl = "";

    // set Version in Help tab
    // lblVersion->setText(QString("VERSION %1").arg(VERSION));

    // Set up touch on all widgets' viewports which inherit from QAbstractScrollArea
    // https://doc.qt.io/qt-6/qtouchevent.html#details
    // scrollArea->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    // txvHelp->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    // txvAbout->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    // QScroller::grabGesture(scrollArea, QScroller::TouchGesture);
    // QScroller::grabGesture(txvHelp, QScroller::TouchGesture);
    // QScroller::grabGesture(txvAbout, QScroller::TouchGesture);

    // set Test Mode
    // devsetting1->setText(pSettings->strTestMode);

    // Add help text to controls -----------------------------------------------
    // input level meter
    QString strInpLevH = "<b>" + tr ( "Input Level Meter" ) + ":</b> " +
                         tr ( "This shows "
                              "the level of the two stereo channels "
                              "for your audio input." ) +
                         "<br>" +
                         tr ( "Make sure not to clip the input signal to avoid distortions of the "
                              "audio signal." );

    QString strInpLevHTT = tr ( "Check input activity here." ) +
                           "<br>" +
                           tr ( "Mute your own input if need be in the session mixer." ) +
                           TOOLTIP_COM_END_TEXT;

    QString strInpLevHAccText  = tr ( "Input level meter" );
    QString strInpLevHAccDescr = tr ( "Simulates an analog LED level meter." );

    lblInputLEDMeter->setWhatsThis ( strInpLevH );
    lblLevelMeterLeft->setWhatsThis ( strInpLevH );
    lblLevelMeterRight->setWhatsThis ( strInpLevH );
    lbrInputLevelL->setWhatsThis ( strInpLevH );
    lbrInputLevelL->setAccessibleName ( strInpLevHAccText );
    lbrInputLevelL->setAccessibleDescription ( strInpLevHAccDescr );
    lbrInputLevelL->setToolTip ( strInpLevHTT );
    lbrInputLevelL->setEnabled ( false );
    lbrInputLevelR->setWhatsThis ( strInpLevH );
    lbrInputLevelR->setAccessibleName ( strInpLevHAccText );
    lbrInputLevelR->setAccessibleDescription ( strInpLevHAccDescr );
    lbrInputLevelR->setToolTip ( strInpLevHTT );
    lbrInputLevelR->setEnabled ( false );

    // connect/disconnect button
    butConnect->setWhatsThis ( "<b>" + tr ( "Connect/Disconnect Button" ) + ":</b> " +
                               tr ( "Opens a dialog where you can select a server to connect to. "
                                    "If you are connected, pressing this button will end the session." ) );

    butConnect->setAccessibleName ( tr ( "Connect and disconnect toggle button" ) );

    // connect/disconnect button
    butNewStart->setWhatsThis ( "<b>" + tr ( "Start New Session Button" ) + ":</b> " +
                               tr ( "Opens default browser at Koord.Live to start a new private session." ) );

    butNewStart->setAccessibleName ( tr ( "Start New Private Session button" ) );

    // reverberation level
    QString strAudReverb = "<b>" + tr ( "Reverb effect" ) + ":</b> " +
                           tr ( "Reverb can be applied to one local mono audio channel or to both "
                                "channels in stereo mode. The mono channel selection and the "
                                "reverb level can be modified. For example, if "
                                "a microphone signal is fed in to the right audio channel of the "
                                "sound card and a reverb effect needs to be applied, set the "
                                "channel selector to right and move the fader upwards until the "
                                "desired reverb level is reached." );

   lblAudioReverb->setWhatsThis ( strAudReverb );
   sldAudioReverb->setWhatsThis ( strAudReverb );

   sldAudioReverb->setAccessibleName ( tr ( "Reverb effect level setting" ) );

    // reverberation channel selection
    QString strRevChanSel = "<b>" + tr ( "Reverb Channel Selection" ) + ":</b> " +
                            tr ( "With these radio buttons the audio input channel on which the "
                                 "reverb effect is applied can be chosen. Either the left "
                                 "or right input channel can be selected." );

   rbtReverbSelL->setWhatsThis ( strRevChanSel );
   rbtReverbSelL->setAccessibleName ( tr ( "Left channel selection for reverb" ) );
   rbtReverbSelR->setWhatsThis ( strRevChanSel );
   rbtReverbSelR->setAccessibleName ( tr ( "Right channel selection for reverb" ) );

    // delay LED
    QString strLEDDelay = "<b>" + tr ( "Delay Status LED" ) + ":</b> " + tr ( "Shows the current audio delay status:" ) +
                          "<ul>"
                          "<li>"
                          "<b>" +
                          tr ( "Green" ) + ":</b> " +
                          tr ( "The delay is perfect for a jam "
                               "session." ) +
                          "</li>"
                          "<li>"
                          "<b>" +
                          tr ( "Yellow" ) + ":</b> " +
                          tr ( "A session is still possible "
                               "but it may be harder to play." ) +
                          "</li>"
                          "<li>"
                          "<b>" +
                          tr ( "Red" ) + ":</b> " +
                          tr ( "The delay is too large for "
                               "jamming." ) +
                          "</li>"
                          "</ul>";

    lblDelay->setWhatsThis ( strLEDDelay );
    ledDelay->setWhatsThis ( strLEDDelay );
    ledDelay->setToolTip ( tr ( "If this LED indicator turns red, "
                                "you will not have much fun using %1." )
                               .arg ( APP_NAME ) +
                           TOOLTIP_COM_END_TEXT );

    ledDelay->setAccessibleName ( tr ( "Delay status LED indicator" ) );

    // buffers LED
    QString strLEDBuffers = "<b>" + tr ( "Local Jitter Buffer Status LED" ) + ":</b> " +
                            tr ( "The local jitter buffer status LED shows the current audio/streaming "
                                 "status. If the light is red, the audio stream is interrupted. "
                                 "This is caused by one of the following problems:" ) +
                            "<ul>"
                            "<li>" +
                            tr ( "The network jitter buffer is not large enough for the current "
                                 "network/audio interface jitter." ) +
                            "</li>"
                            "<li>" +
                            tr ( "The sound card's buffer delay (buffer size) is too small "
                                 "(see Settings window)." ) +
                            "</li>"
                            "<li>" +
                            tr ( "The upload or download stream rate is too high for your "
                                 "internet bandwidth." ) +
                            "</li>"
                            "<li>" +
                            tr ( "The CPU of the client or server is at 100%." ) +
                            "</li>"
                            "</ul>";

    lblBuffers->setWhatsThis ( strLEDBuffers );
    ledBuffers->setWhatsThis ( strLEDBuffers );
    ledBuffers->setToolTip ( tr ( "If this LED indicator turns red, "
                                  "the audio stream is interrupted." ) +
                             TOOLTIP_COM_END_TEXT );

    ledBuffers->setAccessibleName ( tr ( "Local Jitter Buffer status LED indicator" ) );

    // current connection status details
    QString strConnStats = "<b>" + tr ( "Current Connection Status" ) + ":</b> " +
                           tr ( "The Ping Time is the time required for the audio "
                                "stream to travel from the client to the server and back again. This "
                                "delay is introduced by the network and should be about "
                                "20-30 ms. If this delay is higher than about 50 ms, your distance to "
                                "the server is too large or your internet connection is not "
                                "sufficient." ) +
                           "<br>" +
                           tr ( "Overall Delay is calculated from the current Ping Time and the "
                                "delay introduced by the current buffer settings." );

    lblPing->setWhatsThis ( strConnStats );
    lblPingVal->setWhatsThis ( strConnStats );
    lblDelay->setWhatsThis ( strConnStats );
    lblDelayVal->setWhatsThis ( strConnStats );
    lblPingVal->setText ( "---" );
    lblPingUnit->setText ( "" );
    lblDelayVal->setText ( "---" );
    lblDelayUnit->setText ( "" );

    // init GUI design
    SetGUIDesign ( pClient->GetGUIDesign() );

    // MeterStyle init
    SetMeterStyle ( pClient->GetMeterStyle() );

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
    OnTimerStatus();

    // init connection button text
    butConnect->setText ( tr ( "Join" ) );

    // init input level meter bars
    lbrInputLevelL->SetValue ( 0 );
    lbrInputLevelR->SetValue ( 0 );

    // init status LEDs
    ledBuffers->Reset();
    ledDelay->Reset();

    // init audio reverberation
    sldAudioReverb->setRange ( 0, AUD_REVERB_MAX );
    const int iCurAudReverb = pClient->GetReverbLevel();
    sldAudioReverb->setValue ( iCurAudReverb );

    // init input boost
    pClient->SetInputBoost ( pSettings->iInputBoost );

    // init reverb channel
    UpdateRevSelection();

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
    QObject::connect ( downloadLinkButton, &QPushButton::clicked, this, &CClientDlg::OnDownloadUpdateClicked );
    QObject::connect ( inviteComboBox, &QComboBox::activated, this, &CClientDlg::OnInviteBoxActivated );
    QObject::connect ( checkUpdateButton, &QPushButton::clicked, this, &CClientDlg::OnCheckForUpdate );

    // connection for macOS custom url event handler
    QObject::connect ( this, &CClientDlg::EventJoinConnectClicked, this, &CClientDlg::OnEventJoinConnectClicked );

    // check boxes
    QObject::connect ( chbSettings, &QCheckBox::stateChanged, this, &CClientDlg::OnSettingsStateChanged );

//    QObject::connect ( chbPubJam, &QCheckBox::stateChanged, this, &CClientDlg::OnPubConnectStateChanged );

    QObject::connect ( chbChat, &QCheckBox::stateChanged, this, &CClientDlg::OnChatStateChanged );

    QObject::connect ( chbLocalMute, &QCheckBox::stateChanged, this, &CClientDlg::OnLocalMuteStateChanged );

    // Connections -------------------------------------------------------------
    // list view
//    QObject::connect ( lvwServers, &QTreeWidget::itemDoubleClicked, this, &CClientDlg::OnServerListItemDoubleClicked );

    // to get default return key behaviour working
//    QObject::connect ( lvwServers, &QTreeWidget::activated, this, &CClientDlg::OnConnectClicked );

    // timers
    QObject::connect ( &TimerSigMet, &QTimer::timeout, this, &CClientDlg::OnTimerSigMet );

    QObject::connect ( &TimerBuffersLED, &QTimer::timeout, this, &CClientDlg::OnTimerBuffersLED );

    QObject::connect ( &TimerStatus, &QTimer::timeout, this, &CClientDlg::OnTimerStatus );

    QObject::connect ( &TimerPing, &QTimer::timeout, this, &CClientDlg::OnTimerPing );

    QObject::connect ( &RegionTimerPing, &QTimer::timeout, this, &CClientDlg::OnRegionTimerPing );

    QObject::connect ( &TimerReRequestServList, &QTimer::timeout, this, &CClientDlg::OnTimerReRequestServList );

    QObject::connect ( &TimerCheckAudioDeviceOk, &QTimer::timeout, this, &CClientDlg::OnTimerCheckAudioDeviceOk );

    QObject::connect ( &TimerDetectFeedback, &QTimer::timeout, this, &CClientDlg::OnTimerDetectFeedback );

    QObject::connect ( sldAudioReverb, &QDial::valueChanged, this, &CClientDlg::OnAudioReverbValueChanged );

    // radio buttons
    QObject::connect ( rbtReverbSelL, &QRadioButton::clicked, this, &CClientDlg::OnReverbSelLClicked );

    QObject::connect ( rbtReverbSelR, &QRadioButton::clicked, this, &CClientDlg::OnReverbSelRClicked );

    // other
    QObject::connect ( pClient, &CClient::ConClientListMesReceived, this, &CClientDlg::OnConClientListMesReceived );

    QObject::connect ( pClient, &CClient::Disconnected, this, &CClientDlg::OnDisconnected );

    QObject::connect ( pClient, &CClient::ClientIDReceived, this, &CClientDlg::OnClientIDReceived );

    QObject::connect ( pClient, &CClient::MuteStateHasChangedReceived, this, &CClientDlg::OnMuteStateHasChangedReceived );

    QObject::connect ( pClient, &CClient::RecorderStateReceived, this, &CClientDlg::OnRecorderStateReceived );

    QObject::connect ( pClient, &CClient::PingTimeReceived, this, &CClientDlg::OnPingTimeResult );

    QObject::connect ( pClient, &CClient::CLServerListReceived, this, &CClientDlg::OnCLServerListReceived );

    QObject::connect ( pClient, &CClient::CLRedServerListReceived, this, &CClientDlg::OnCLRedServerListReceived );

    QObject::connect ( pClient, &CClient::CLConnClientsListMesReceived, this, &CClientDlg::OnCLConnClientsListMesReceived );

    QObject::connect ( pClient, &CClient::CLPingTimeWithNumClientsReceived, this, &CClientDlg::OnCLPingTimeWithNumClientsReceived );

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

    QObject::connect ( this, &CClientDlg::ReqServerListQuery, this, &CClientDlg::OnReqServerListQuery );

    QObject::connect ( sessionCancelButton, &QPushButton::clicked, this, &CClientDlg::OnJoinCancelClicked );
    QObject::connect ( sessionConnectButton, &QPushButton::clicked, this, &CClientDlg::OnJoinConnectClicked );

    // note that this connection must be a queued connection, otherwise the server list ping
    // times are not accurate and the client list may not be retrieved for all servers listed
    // (it seems the sendto() function needs to be called from different threads to fire the
    // packet immediately and do not collect packets before transmitting)
    QObject::connect ( this, &CClientDlg::CreateCLServerListPingMes, this, &CClientDlg::OnCreateCLServerListPingMes, Qt::QueuedConnection );

    QObject::connect ( this, &CClientDlg::CreateCLServerListReqVerAndOSMes, this, &CClientDlg::OnCreateCLServerListReqVerAndOSMes );

    QObject::connect ( this,
                       &CClientDlg::CreateCLServerListReqConnClientsListMes,
                       this,
                       &CClientDlg::OnCreateCLServerListReqConnClientsListMes );


//     // ==================================================================================================
//     // SETTINGS SLOTS ========
//     // ==================================================================================================
//     // Connections -------------------------------------------------------------
//     // timers
//     QObject::connect ( &TimerStatus, &QTimer::timeout, this, &CClientDlg::OnTimerStatus );

//     // slider controls
//     QObject::connect ( sldNetBuf, &QSlider::valueChanged, this, &CClientDlg::OnNetBufValueChanged );

//     QObject::connect ( sldNetBufServer, &QSlider::valueChanged, this, &CClientDlg::OnNetBufServerValueChanged );

//     // check boxes
//     QObject::connect ( chbAutoJitBuf, &QCheckBox::stateChanged, this, &CClientDlg::OnAutoJitBufStateChanged );

//     QObject::connect ( chbEnableOPUS64, &QCheckBox::stateChanged, this, &CClientDlg::OnEnableOPUS64StateChanged );

//     QObject::connect ( chbDetectFeedback, &QCheckBox::stateChanged, this, &CClientDlg::OnFeedbackDetectionChanged );

//     QObject::connect ( newInputLevelDial,
//                        &QSlider::valueChanged,
//                        this,
//                        &CClientDlg::OnNewClientLevelChanged );

//     // combo boxes
//     QObject::connect ( cbxSoundcard,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnSoundcardActivated );
   
//     QObject::connect ( cbxLInChan,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnLInChanActivated );

//     QObject::connect ( cbxRInChan,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnRInChanActivated );

//     QObject::connect ( cbxLOutChan,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnLOutChanActivated );

//     QObject::connect ( cbxROutChan,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnROutChanActivated );

//     QObject::connect ( cbxAudioChannels,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnAudioChannelsActivated );

//     QObject::connect ( cbxAudioQuality,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnAudioQualityActivated );

//     QObject::connect ( cbxSkin,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnGUIDesignActivated );

//     QObject::connect ( cbxMeterStyle,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnMeterStyleActivated );

//     QObject::connect ( cbxCustomDirectories->lineEdit(), &QLineEdit::editingFinished, this, &CClientDlg::OnCustomDirectoriesEditingFinished );

//     QObject::connect ( cbxCustomDirectories,
//                        static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                        this,
//                        &CClientDlg::OnCustomDirectoriesEditingFinished );

//     QObject::connect ( cbxLanguage, &CLanguageComboBox::LanguageChanged, this, &CClientDlg::OnLanguageChanged );

//     QObject::connect ( dialInputBoost,
//                        &QSlider::valueChanged,
//                        this,
//                        &CClientDlg::OnInputBoostChanged );

//     // buttons
// #if defined( _WIN32 ) && !defined( WITH_JACK )
//     // Driver Setup button is only available for Windows when JACK is not used
//     QObject::connect ( butDriverSetup, &QPushButton::clicked, this, &CClientDlg::OnDriverSetupClicked );
//     // make driver refresh button reload the currently selected sound card
// //    QObject::connect ( driverRefresh, &QPushButton::clicked, this, &CClientDlg::OnSoundcardReactivate );
// #endif

//     // misc
//     // sliders
//     // panDial
//     QObject::connect ( panDial, &QSlider::valueChanged, this, &CClientDlg::OnAudioPanValueChanged );

//     QObject::connect ( &SndCrdBufferDelayButtonGroup,
//                        static_cast<void ( QButtonGroup::* ) ( QAbstractButton* )> ( &QButtonGroup::buttonClicked ),
//                        this,
//                        &CClientDlg::OnSndCrdBufferDelayButtonGroupClicked );

//     // spinners
//     QObject::connect ( spnMixerRows,
//                        static_cast<void ( QSpinBox::* ) ( int )> ( &QSpinBox::valueChanged ),
//                        this,
//                        &CClientDlg::NumMixerPanelRowsChanged );

//     // Musician Profile
//     QObject::connect ( pedtAlias, &QLineEdit::textChanged, this, &CClientDlg::OnAliasTextChanged );

// //    QObject::connect ( pcbxInstrument,
// //                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
// //                       this,
// //                       &CClientDlg::OnInstrumentActivated );

// //    QObject::connect ( pcbxCountry,
// //                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
// //                       this,
// //                       &CClientDlg::OnCountryActivated );

// //    QObject::connect ( pedtCity, &QLineEdit::textChanged, this, &CClientDlg::OnCityTextChanged );

// //    QObject::connect ( pcbxSkill, static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ), this, &CClientDlg::OnSkillActivated );


//     // Timers ------------------------------------------------------------------
//     // start timer for status bar
//     TimerStatus.start ( DISPLAY_UPDATE_TIME );

//     // END OF SETTINGS SLOTS =======================================================================================


    // Initializations which have to be done after the signals are connected ---
    // start timer for status bar
    TimerStatus.start ( LED_BAR_UPDATE_TIME_MS );

    sessionJoinWidget->setVisible(false);

    // mute stream on startup (must be done after the signal connections)
    if ( bMuteStream )
    {
        chbLocalMute->setCheckState ( Qt::Checked );
    }

    // query the update server version number needed for update check (note
    // that the connection less message respond may not make it back but that
    // is not critical since the next time Jamulus is started we have another
    // chance and the update check is not time-critical at all)
    CHostAddress UpdateServerHostAddress;

    // do Koord version check - non-appstore versions only
    // check for existence of INSTALL_DIR/nonstore_donotdelete.txt

    // check for existence of update-checker flagfile - to NOT use in stores (Apple in particular doesn't like)
//    QString check_file_path = QApplication::applicationDirPath() +  "/nonstore_donotdelete.txt";
    QFileInfo check_file(QApplication::applicationDirPath() +  "/nonstore_donotdelete.txt");
    if (check_file.exists() && check_file.isFile()) {
        OnCheckForUpdate();
    }

    // Send the request to two servers for redundancy if either or both of them
    // has a higher release version number, the reply will trigger the notification.

    if ( NetworkUtil().ParseNetworkAddress ( UPDATECHECK1_ADDRESS, UpdateServerHostAddress, bEnableIPv6 ) )
    {
        pClient->CreateCLServerListReqVerAndOSMes ( UpdateServerHostAddress );
    }

    if ( NetworkUtil().ParseNetworkAddress ( UPDATECHECK2_ADDRESS, UpdateServerHostAddress, bEnableIPv6 ) )
    {
        pClient->CreateCLServerListReqVerAndOSMes ( UpdateServerHostAddress );
    }
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

void CClientDlg::ManageDragNDrop ( QDropEvent* Event, const bool bCheckAccept )
{
    // we only want to use drag'n'drop with file URLs
    QListIterator<QUrl> UrlIterator ( Event->mimeData()->urls() );

    while ( UrlIterator.hasNext() )
    {
        const QString strFileNameWithPath = UrlIterator.next().toLocalFile();

        // check all given URLs and if any has the correct suffix
        if ( !strFileNameWithPath.isEmpty() && ( QFileInfo ( strFileNameWithPath ).suffix() == MIX_SETTINGS_FILE_SUFFIX ) )
        {
            if ( bCheckAccept )
            {
                // only accept drops of supports file types
                Event->acceptProposedAction();
            }
            else
            {
                // load the first valid settings file and leave the loop
                pSettings->LoadFaderSettings ( strFileNameWithPath );
                MainMixerBoard->LoadAllFaderSettings();
                break;
            }
        }
    }
}

void CClientDlg::UpdateRevSelection()
{
    if ( pClient->GetAudioChannels() == CC_STEREO )
    {
        // for stereo make channel selection invisible since
        // reverberation effect is always applied to both channels
       rbtReverbSelL->setVisible ( false );
       rbtReverbSelR->setVisible ( false );
    }
    else
    {
        // make radio buttons visible
       rbtReverbSelL->setVisible ( true );
       rbtReverbSelR->setVisible ( true );

        // update value
        if ( pClient->IsReverbOnLeftChan() )
        {
           rbtReverbSelL->setChecked ( true );
        }
        else
        {
           rbtReverbSelR->setChecked ( true );
        }
    }

    // update visibility of the pan controls in the audio mixer board (pan is not supported for mono)
    MainMixerBoard->SetDisplayPans ( pClient->GetAudioChannels() != CC_MONO );
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
        SetMixerBoardDeco ( RS_UNDEFINED, pClient->GetGUIDesign() );
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
    if ( lvwServers->topLevelItem ( 0 )->text ( 0 ).contains("Directory") )  {
        idx = 1;
    }
    strCurrBestRegion = lvwServers->topLevelItem ( idx )->text ( 0 )
                            .replace( matchState, "" )  // remove any state refs eg TX or DC
                            .replace( " ", "" )            // remove remaining whitespace eg in "New York"
                            .replace( ",", "" )            // remove commas, prob before
                            .replace( "Zürich", "Zurich" )      // special case #1
                            .replace( "SãoPaulo", "SaoPaulo" )  // special case #2
                            .toLower();
    // qInfo() << strCurrBestRegion;
    QDesktopServices::openUrl(QUrl("https://koord.live/session?region=" + strCurrBestRegion, QUrl::TolerantMode));
}

void CClientDlg::OnDownloadUpdateClicked()
{
    // just open website for now
    QDesktopServices::openUrl(QUrl("https://koord.live/downloads", QUrl::TolerantMode));
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

void CClientDlg::OnSettingsStateChanged ( int value )
{
    if ( value == Qt::Checked )
    {
        ;
//        ShowGeneralSettings ( SETTING_TAB_AUDIONET );
    }
    else
    {
        ;
//        ClientSettingsDlg.hide();
    }
}


void CClientDlg::OnChatStateChanged ( int value )
{
    if ( value == Qt::Checked )
    {
        // do nothing right now
        ;
//        ShowChatWindow();
    }
    else
    {
        // do nothing right now
        ;
     //   ChatDlg.hide();
    }
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

void CClientDlg::OnCLPingTimeWithNumClientsReceived ( CHostAddress InetAddr, int iPingTime, int iNumClients )
{
    // update connection dialog server list
    SetPingTimeAndNumClientsResult ( InetAddr, iPingTime, iNumClients );
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
        // stop region ping timer
        RegionTimerPing.stop();
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
    RegionTimerPing.start();
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
    OnTimerStatus();
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

void CClientDlg::OnCheckForUpdate()
{
    // read from Github API to get latest release ie tag rX_X_X
    QUrl url("https://api.github.com/repos/koord-live/koord-app/releases");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // send request and assign reply pointer
    QNetworkReply *reply = qNam->get(request);

    // connect reply pointer with callback for finished signal
    QObject::connect(reply, &QNetworkReply::finished, this, [=]()
        {
            // DL installer URL to download if required
            QString new_download_url;
            QString err = reply->errorString();
            QString contents = QString::fromUtf8(reply->readAll());
            if ( reply->error() != QNetworkReply::NoError)
            {
                // QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "Network Error!" );
                return;
            }
            //qInfo() << ">>> CONNECT: " << reply->error();

            QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
            QJsonArray jsonArray = jsonResponse.array();

            for (int jsIndex = 0; jsIndex < jsonArray.size(); ++jsIndex) {
                QJsonObject releaseObj = jsonArray[jsIndex].toObject();
                foreach(const QString& key, releaseObj.keys()) {
                    if ( key == "tag_name")
                    {
                        QJsonValue value = releaseObj.value(key);
                        qInfo() << "Key = " << key << ", Value = " << value.toString();
                        QRegularExpression rx_release("^r[0-9]+_[0-9]+_[0-9]+$");
                        QRegularExpressionMatch rel_match = rx_release.match(value.toString());
                        if (rel_match.hasMatch()) {
                            QString latestVersion =  rel_match.captured(0)
                                                       .replace("r", "")
                                                       .replace("_", ".");
                            if ( APP_VERSION == latestVersion )
                            {
                                qInfo() << "WE HAVE A MATCH - no update necessary";
                                // QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "Up to date!" );
                            }
                            else
                            {
                                qInfo() << "Later version available: " << latestVersion;
                                // get download URL
                                QJsonArray DLArray = releaseObj.value("assets").toArray();
                                qInfo() << DLArray.size();
                                for (int dlIndex = 0; dlIndex < DLArray.size(); ++dlIndex) {
                                    QJsonObject dlObject =  DLArray[dlIndex].toObject();
                                    foreach(const QString& dlkey, dlObject.keys()) {
                                        if ( dlkey == "browser_download_url") {
                                            QString dl_url = dlObject.value(dlkey).toString();
#if defined( Q_OS_MACOS ) && defined ( MAC_LEGACY )
                                            if (dl_url.endsWith("legacy.dmg")) new_download_url = dl_url;
#elif defined( Q_OS_MACOS ) && !defined ( MAC_LEGACY )
                                            if (dl_url.endsWith(".dmg") && !dl_url.contains("legacy")) new_download_url = dl_url;
#elif defined ( Q_OS_WINDOWS )
                                            if (dl_url.endsWith(".exe")) new_download_url = dl_url;
#elif defined ( Q_OS_LINUX )
                                            if (dl_url.endsWith(".AppImage")) new_download_url = dl_url;
#endif
                                        }
                                    }
                                }
                                QMessageBox updateMessage = QMessageBox(this);
                                updateMessage.setText(QString("There is an update available! Version: %1").arg(latestVersion));
                                updateMessage.setInformativeText(QString("It's highly recommended to stay up to date "
                                                                         "with all the fixes and enhancements. Shall we get the update?"));
                                QPushButton *dlButton = updateMessage.addButton(QString("Download Update"), QMessageBox::AcceptRole);
                                QPushButton *cancelButton = updateMessage.addButton(QString("Later"), QMessageBox::RejectRole);
                                updateMessage.setDefaultButton(dlButton);
                                updateMessage.exec();
                                if (updateMessage.clickedButton() == dlButton) {
                                    QDesktopServices::openUrl(QUrl(new_download_url, QUrl::TolerantMode));
                                } else if (updateMessage.clickedButton() == cancelButton) {
                                    return;
                                }

                            };
                            // IF we have a match, then that is the latest version
                            // Github returns array with latest releases at start of index
                            // So return after first successful match
                            return;
                        };
                    }
                }
            }
//            QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "No Update Found" );
        });

}

void CClientDlg::UpdateDisplay()
{
    // nothing
    ;
}

void CClientDlg::SetGUIDesign ( const EGUIDesign eNewDesign )
{
    // remove any styling from the mixer board - reapply after changing skin
    MainMixerBoard->setStyleSheet ( "" );

    // apply GUI design to current window
    switch ( eNewDesign )
    {
    case GD_ORIGINAL:
        backgroundFrame->setStyleSheet (
            "QFrame#backgroundFrame { border-image:  url(:/png/main/res/background.png) 0px 0px 0px 0px;"
            "                         border-top:    0px transparent;"
            "                         border-bottom: 0px transparent;"
            "                         border-left:   0px transparent;"
            "                         border-right:  0px transparent;"
            "                         padding:       2px;"
            "                         margin:        2px, 2px, 2px, 2px; }"
            "QLabel {                 color:          rgb(220, 220, 220);"
            "                         font:           Rubik; }"
            "QRadioButton {           color:          rgb(220, 220, 220);"
            "                         font:           bold; }"
            "QScrollArea {            background:     transparent; }"
            ".QWidget {               background:     transparent; }" // note: matches instances of QWidget, but not of its subclasses
            "QGroupBox {              background:     transparent; }"
            "QGroupBox::title {       color:          rgb(220, 220, 220); }"
            "QCheckBox::indicator {   width:          38px;"
            "                         height:         21px; }"
            "QCheckBox::indicator:unchecked {"
            "                         image:          url(:/png/main/res/general_btn_off.png); }"
            "QCheckBox::indicator:checked {"
            "                         image:          url(:/png/main/res/general_btn_on.png); }"
            "QCheckBox {              color:          rgb(220, 220, 220);"
            "                         font:           bold; }" );

#ifdef _WIN32
       // Workaround QT-Windows problem: This should not be necessary since in the
       // background frame the style sheet for QRadioButton was already set. But it
       // seems that it is only applied if the style was set to default and then back
       // to GD_ORIGINAL. This seems to be a QT related issue...
       rbtReverbSelL->setStyleSheet ( "color: rgb(220, 220, 220);"
                                      "font:  bold;" );
       rbtReverbSelR->setStyleSheet ( "color: rgb(220, 220, 220);"
                                      "font:  bold;" );
#endif

        ledBuffers->SetType ( CMultiColorLED::MT_LED );
        ledDelay->SetType ( CMultiColorLED::MT_LED );
        break;

    default:
        // reset style sheet and set original parameters
        backgroundFrame->setStyleSheet ( "" );

#ifdef _WIN32
       // Workaround QT-Windows problem: See above description
       rbtReverbSelL->setStyleSheet ( "" );
       rbtReverbSelR->setStyleSheet ( "" );
#endif

        ledBuffers->SetType ( CMultiColorLED::MT_INDICATOR );
        ledDelay->SetType ( CMultiColorLED::MT_INDICATOR );
        break;
    }

    // also apply GUI design to child GUI controls
    MainMixerBoard->SetGUIDesign ( eNewDesign );
}

void CClientDlg::SetMeterStyle ( const EMeterStyle eNewMeterStyle )
{
    // apply MeterStyle to current window
    switch ( eNewMeterStyle )
    {
    case MT_LED_STRIPE:
        lbrInputLevelL->SetLevelMeterType ( CLevelMeter::MT_LED_STRIPE );
        lbrInputLevelR->SetLevelMeterType ( CLevelMeter::MT_LED_STRIPE );
        break;

    case MT_LED_ROUND_BIG:
        lbrInputLevelL->SetLevelMeterType ( CLevelMeter::MT_LED_ROUND_BIG );
        lbrInputLevelR->SetLevelMeterType ( CLevelMeter::MT_LED_ROUND_BIG );
        break;

    case MT_BAR_WIDE:
        lbrInputLevelL->SetLevelMeterType ( CLevelMeter::MT_BAR_WIDE );
        lbrInputLevelR->SetLevelMeterType ( CLevelMeter::MT_BAR_WIDE );
        break;

    case MT_BAR_NARROW:
        lbrInputLevelL->SetLevelMeterType ( CLevelMeter::MT_BAR_WIDE );
        lbrInputLevelR->SetLevelMeterType ( CLevelMeter::MT_BAR_WIDE );
        break;

    case MT_LED_ROUND_SMALL:
        lbrInputLevelL->SetLevelMeterType ( CLevelMeter::MT_LED_ROUND_BIG );
        lbrInputLevelR->SetLevelMeterType ( CLevelMeter::MT_LED_ROUND_BIG );
        break;
    }

    // also apply MeterStyle to child GUI controls
    MainMixerBoard->SetMeterStyle ( eNewMeterStyle );
}

void CClientDlg::OnRecorderStateReceived ( const ERecorderState newRecorderState )
{
    MainMixerBoard->SetRecorderState ( newRecorderState );
    SetMixerBoardDeco ( newRecorderState, pClient->GetGUIDesign() );
}

void CClientDlg::OnGUIDesignChanged()
{
    SetGUIDesign ( pClient->GetGUIDesign() );
    SetMixerBoardDeco ( MainMixerBoard->GetRecorderState(), pClient->GetGUIDesign() );
}

void CClientDlg::OnMeterStyleChanged() { SetMeterStyle ( pClient->GetMeterStyle() ); }

void CClientDlg::SetMixerBoardDeco ( const ERecorderState newRecorderState, const EGUIDesign eNewDesign )
{
    // return if no change
    if ( ( newRecorderState == eLastRecorderState ) && ( eNewDesign == eLastDesign ) )
        return;
    eLastRecorderState = newRecorderState;
    eLastDesign        = eNewDesign;

    if ( newRecorderState == RS_RECORDING )
    {
        MainMixerBoard->setStyleSheet ( "QGroupBox::title { subcontrol-origin: margin; "
                                        "                   subcontrol-position: left top;"
                                        "                   left: 7px;"
                                        "                   color: rgb(255,255,255);"
                                        "                   background-color: rgb(255,0,0); }" );
        recLabel->setStyleSheet ( "QLabel { color: red; font: bold; }" );

    }
    else
    {
        if ( eNewDesign == GD_ORIGINAL )
        {
            MainMixerBoard->setStyleSheet ( "QGroupBox::title { subcontrol-origin: margin;"
                                            "                   subcontrol-position: left top;"
                                            "                   left: 7px;"
                                            "                   color: rgb(220,220,220); }" );
        }
        else
        {
            MainMixerBoard->setStyleSheet ( "QGroupBox::title { subcontrol-origin: margin;"
                                            "                   subcontrol-position: left top;"
                                            "                   left: 7px;"
                                            "                   color: rgb(0,0,0); }" );
        }
        recLabel->setStyleSheet ( "QLabel { color: rgb(86, 86, 86); font: normal; }" );
    }
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


// Region Checker stuff
void CClientDlg::RequestServerList()
{
    // reset flags
    bServerListReceived        = false;
    bReducedServerListReceived = false;
    bServerListItemWasChosen   = false;
    bListFilterWasActive       = false;

    // clear current address and name
//    strSelectedAddress    = "";
//    strSelectedServerName = "";

    // clear server list view
    lvwServers->clear();

    // update list combo box (disable events to avoid a signal)
    cbxDirectoryServer->blockSignals ( true );
    if ( pSettings->eDirectoryType == AT_CUSTOM )
    {
        // iCustomDirectoryIndex is non-zero only if eDirectoryType == AT_CUSTOM
        // find the combobox item that corresponds to vstrDirectoryAddress[iCustomDirectoryIndex]
        // (the current selected custom directory)
        cbxDirectoryServer->setCurrentIndex ( cbxDirectoryServer->findData ( QVariant ( pSettings->iCustomDirectoryIndex ) ) );
    }
    else
    {
        cbxDirectoryServer->setCurrentIndex ( static_cast<int> ( pSettings->eDirectoryType ) );
    }
    cbxDirectoryServer->blockSignals ( false );

    // Get the IP address of the directory server (using the ParseNetworAddress
    // function) when the connect dialog is opened, this seems to be the correct
    // time to do it. Note that in case of custom directories we
    // use iCustomDirectoryIndex as an index into the vector.

    // Allow IPv4 only for communicating with Directories
    if ( NetworkUtil().ParseNetworkAddress (
             NetworkUtil::GetDirectoryAddress ( pSettings->eDirectoryType, pSettings->vstrDirectoryAddress[pSettings->iCustomDirectoryIndex] ),
             haDirectoryAddress,
             false ) )
    {
        // send the request for the server list
        emit ReqServerListQuery ( haDirectoryAddress );

        // start timer, if this message did not get any respond to retransmit
        // the server list request message
        TimerReRequestServList.start ( SERV_LIST_REQ_UPDATE_TIME_MS );
        TimerInitialSort.start ( SERV_LIST_REQ_UPDATE_TIME_MS ); // reuse the time value
    }
}



void CClientDlg::OnDirectoryServerChanged ( int iTypeIdx )
{
    // store the new directory type and request new list
    // if iTypeIdx == AT_CUSTOM, then iCustomDirectoryIndex is the index into the vector holding the user's custom directory servers
    // if iTypeIdx != AT_CUSTOM, then iCustomDirectoryIndex MUST be 0;
    if ( iTypeIdx >= AT_CUSTOM )
    {
        // the value for the index into the vector vstrDirectoryAddress is in the user data of the combobox item
        pSettings->iCustomDirectoryIndex = cbxDirectoryServer->itemData ( iTypeIdx ).toInt();
        iTypeIdx                         = AT_CUSTOM;
    }
    else
    {
        pSettings->iCustomDirectoryIndex = 0;
    }
    pSettings->eDirectoryType = static_cast<EDirectoryType> ( iTypeIdx );
    RequestServerList();
}

void CClientDlg::OnTimerReRequestServList()
{
    // if the server list is not yet received, retransmit the request for the
    // server list
    if ( !bServerListReceived )
    {
        // note that this is a connection less message which may get lost
        // and therefore it makes sense to re-transmit it
        emit ReqServerListQuery ( haDirectoryAddress );
    }
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

//void CClientDlg::setDefaultSingleUserMode(const QString& value)
//{
//    // Set from URL or command line so string not boolean
//    m_default_single_user_mode = (value.toLower() == "true");
//}

void CClientDlg::SetServerList ( const CHostAddress& InetAddr, const CVector<CServerInfo>& vecServerInfo, const bool bIsReducedServerList )
{
    // If the normal list was received, we do not accept any further list
    // updates (to avoid the reduced list overwrites the normal list (#657)). Also,
    // we only accept a server list from the server address we have sent the
    // request for this to (note that we cannot use the port number since the
    // receive port and send port might be different at the directory server).
    if ( bServerListReceived || ( InetAddr.InetAddr != haDirectoryAddress.InetAddr ) )
    {
        return;
    }

    // special treatment if a reduced server list was received
    if ( bIsReducedServerList )
    {
        // make sure we only apply the reduced version list once
        if ( bReducedServerListReceived )
        {
            // do nothing
            return;
        }
        else
        {
            bReducedServerListReceived = true;
        }
    }
    else
    {
        // set flag and disable timer for resend server list request if full list
        // was received (i.e. not the reduced list)
        bServerListReceived = true;
        TimerReRequestServList.stop();
    }

    // first clear list
    lvwServers->clear();

    // add list item for each server in the server list
    const int iServerInfoLen = vecServerInfo.Size();

    for ( int iIdx = 0; iIdx < iServerInfoLen; iIdx++ )
    {
        // get the host address, note that for the very first entry which is
        // the directory server, we have to use the receive host address
        // instead
        CHostAddress CurHostAddress;

        if ( iIdx > 0 )
        {
            CurHostAddress = vecServerInfo[iIdx].HostAddr;
        }
        else
        {
            // substitute the receive host address for directory server
            CurHostAddress = InetAddr;
        }

        // create new list view item
        QTreeWidgetItem* pNewListViewItem = new QTreeWidgetItem ( lvwServers );

        // make the entry invisible (will be set to visible on successful ping
        // result) if the complete list of registered servers shall not be shown
        if ( !bShowCompleteRegList )
        {
            pNewListViewItem->setHidden ( true );
        }
        // if this is Directory Server, don't show it
        if ( iIdx == 0 )
        {
            pNewListViewItem->setHidden( true );
        }

        // server name (if empty, show host address instead)
        if ( !vecServerInfo[iIdx].strName.isEmpty() )
        {
            pNewListViewItem->setText ( 0, vecServerInfo[iIdx].strName );
        }
        else
        {
            // IP address and port (use IP number without last byte)
            // Definition: If the port number is the default port number, we do
            // not show it.
            if ( vecServerInfo[iIdx].HostAddr.iPort == DEFAULT_PORT_NUMBER )
            {
                // only show IP number, no port number
                pNewListViewItem->setText ( 0, CurHostAddress.toString ( CHostAddress::SM_IP_NO_LAST_BYTE ) );
            }
            else
            {
                // show IP number and port
                pNewListViewItem->setText ( 0, CurHostAddress.toString ( CHostAddress::SM_IP_NO_LAST_BYTE_PORT ) );
            }
        }

        // in case of all servers shown, add the registration number at the beginning
//        if ( bShowCompleteRegList )
//        {
//            pNewListViewItem->setText ( 0, QString ( "%1: " ).arg ( 1 + iIdx, 3 ) + pNewListViewItem->text ( 0 ) );
//        }

        // show server name in bold font if it is a permanent server
//        QFont CurServerNameFont = pNewListViewItem->font ( 0 );
//        CurServerNameFont.setBold ( vecServerInfo[iIdx].bPermanentOnline );
//        pNewListViewItem->setFont ( 0, CurServerNameFont );

        // the ping time shall be shown in bold font
        QFont CurPingTimeFont = pNewListViewItem->font ( 1 );
        CurPingTimeFont.setBold ( true );
        pNewListViewItem->setFont ( 1, CurPingTimeFont );

        // happiness level in emoji font
        pNewListViewItem->setFont ( 2, QFont("Segoe UI Emoji") );

        // server location (city and country)
        QString strLocation = vecServerInfo[iIdx].strCity;

        if ( ( !strLocation.isEmpty() ) && ( vecServerInfo[iIdx].eCountry != QLocale::AnyCountry ) )
        {
            strLocation += ", ";
        }

        if ( vecServerInfo[iIdx].eCountry != QLocale::AnyCountry )
        {
            QString strCountryToString = QLocale::countryToString ( vecServerInfo[iIdx].eCountry );

            // Qt countryToString does not use spaces in between country name
            // parts but they use upper case letters which we can detect and
            // insert spaces as a post processing
            if ( !strCountryToString.contains ( " " ) )
            {
                QRegularExpressionMatchIterator reMatchIt = QRegularExpression ( "[A-Z][^A-Z]*" ).globalMatch ( strCountryToString );
                QStringList                     slNames;
                while ( reMatchIt.hasNext() )
                {
                    slNames << reMatchIt.next().capturedTexts();
                }
                strCountryToString = slNames.join ( " " );
            }
            strLocation += strCountryToString;
        }

        pNewListViewItem->setText ( 3, strLocation );

        // init the minimum ping time with a large number (note that this number
        // must fit in an integer type)
        pNewListViewItem->setText ( 4, "99999999" );

        // store the maximum number of clients
        pNewListViewItem->setText ( 5, QString().setNum ( vecServerInfo[iIdx].iMaxNumClients ) );

        // store host address
        pNewListViewItem->setData ( 0, Qt::UserRole, CurHostAddress.toString() );

        // per default expand the list item (if not "show all servers")
        if ( bShowAllMusicians )
        {
            lvwServers->expandItem ( pNewListViewItem );
        }
    }

    // immediately issue the ping measurements and start the ping timer since
    // the server list is filled now
    OnRegionTimerPing();
    RegionTimerPing.start ( PING_UPDATE_TIME_SERVER_LIST_MS );
}

void CClientDlg::SetConnClientsList ( const CHostAddress& InetAddr, const CVector<CChannelInfo>& vecChanInfo )
{
    // find the server with the correct address
    QTreeWidgetItem* pCurListViewItem = FindListViewItem ( InetAddr );

    if ( pCurListViewItem )
    {
        // first remove any existing children
        DeleteAllListViewItemChilds ( pCurListViewItem );

        // get number of connected clients
        const int iNumConnectedClients = vecChanInfo.Size();

        for ( int i = 0; i < iNumConnectedClients; i++ )
        {
            // create new list view item
            QTreeWidgetItem* pNewChildListViewItem = new QTreeWidgetItem ( pCurListViewItem );

            // child items shall use only one column
            pNewChildListViewItem->setFirstColumnSpanned ( true );

            // set the clients name
            QString sClientText = vecChanInfo[i].strName;

            // set the icon: country flag has priority over instrument
            bool bCountryFlagIsUsed = false;

            if ( vecChanInfo[i].eCountry != QLocale::AnyCountry )
            {
                // try to load the country flag icon
                QPixmap CountryFlagPixmap ( CLocale::GetCountryFlagIconsResourceReference ( vecChanInfo[i].eCountry ) );

                // first check if resource reference was valid
                if ( !CountryFlagPixmap.isNull() )
                {
                    // set correct picture
                    pNewChildListViewItem->setIcon ( 0, QIcon ( CountryFlagPixmap ) );

                    bCountryFlagIsUsed = true;
                }
            }

            if ( !bCountryFlagIsUsed )
            {
                // get the resource reference string for this instrument
                const QString strCurResourceRef = CInstPictures::GetResourceReference ( vecChanInfo[i].iInstrument );

                // first check if instrument picture is used or not and if it is valid
                if ( !( CInstPictures::IsNotUsedInstrument ( vecChanInfo[i].iInstrument ) || strCurResourceRef.isEmpty() ) )
                {
                    // set correct picture
                    pNewChildListViewItem->setIcon ( 0, QIcon ( QPixmap ( strCurResourceRef ) ) );
                }
            }

            // add the instrument information as text
            if ( !CInstPictures::IsNotUsedInstrument ( vecChanInfo[i].iInstrument ) )
            {
                sClientText.append ( " (" + CInstPictures::GetName ( vecChanInfo[i].iInstrument ) + ")" );
            }

            // apply the client text to the list view item
            pNewChildListViewItem->setText ( 0, sClientText );

            // add the new child to the corresponding server item
            pCurListViewItem->addChild ( pNewChildListViewItem );

            // at least one server has children now, show decoration to be able
            // to show the children
            lvwServers->setRootIsDecorated ( true );
        }

        // the clients list may have changed, update the filter selection
        UpdateListFilter();
    }
}

//void CClientDlg::OnServerListItemDoubleClicked ( QTreeWidgetItem* Item, int )
//{
//    // if a server list item was double clicked, it is the same as if the
//    // connect button was clicked
//    if ( Item != nullptr )
//    {
//        OnConnectClicked();
//    }
//}

void CClientDlg::OnServerAddrEditTextChanged ( const QString& )
{
    // in the server address combo box, a text was changed, remove selection
    // in the server list (if any)
    lvwServers->clearSelection();
}

void CClientDlg::OnCustomDirectoriesChanged()
{

    QString strPreviousSelection = cbxDirectoryServer->currentText();
    UpdateDirectoryServerComboBox();
    // after updating the combobox, we must re-select the previous directory selection

    if ( pSettings->eDirectoryType == AT_CUSTOM )
    {
        // check if the currently select custom directory still exists in the now potentially re-ordered vector,
        // if so, then change to its new index.  (addresses Issue #1899)
        int iNewIndex = cbxDirectoryServer->findText ( strPreviousSelection, Qt::MatchExactly );
        if ( iNewIndex == INVALID_INDEX )
        {
            // previously selected custom directory has been deleted.  change to default directory
            pSettings->eDirectoryType        = static_cast<EDirectoryType> ( AT_DEFAULT );
            pSettings->iCustomDirectoryIndex = 0;
            RequestServerList();
        }
        else
        {
            // find previously selected custom directory in the now potentially re-ordered vector
            pSettings->eDirectoryType        = static_cast<EDirectoryType> ( AT_CUSTOM );
            pSettings->iCustomDirectoryIndex = cbxDirectoryServer->itemData ( iNewIndex ).toInt();
            cbxDirectoryServer->blockSignals ( true );
            cbxDirectoryServer->setCurrentIndex ( cbxDirectoryServer->findData ( QVariant ( pSettings->iCustomDirectoryIndex ) ) );
            cbxDirectoryServer->blockSignals ( false );
        }
    }
    else
    {
        // selected directory was not a custom directory
        cbxDirectoryServer->blockSignals ( true );
        cbxDirectoryServer->setCurrentIndex ( static_cast<int> ( pSettings->eDirectoryType ) );
        cbxDirectoryServer->blockSignals ( false );
    }
}

//void CClientDlg::ShowAllMusicians ( const bool bState )
//{
//    bShowAllMusicians = bState;

//    // update list
//    if ( bState )
//    {
//        lvwServers->expandAll();
//    }
//    else
//    {
//        lvwServers->collapseAll();
//    }

//    // update check box if necessary
//    if ( ( chbExpandAll->checkState() == Qt::Checked && !bShowAllMusicians ) || ( chbExpandAll->checkState() == Qt::Unchecked && bShowAllMusicians ) )
//    {
//        chbExpandAll->setCheckState ( bState ? Qt::Checked : Qt::Unchecked );
//    }
//}

void CClientDlg::UpdateListFilter()
{
//    const QString sFilterText = edtFilter->text();
    const QString sFilterText = "";

    if ( !sFilterText.isEmpty() )
    {
        bListFilterWasActive     = true;
        const int iServerListLen = lvwServers->topLevelItemCount();

        for ( int iIdx = 0; iIdx < iServerListLen; iIdx++ )
        {
            QTreeWidgetItem* pCurListViewItem = lvwServers->topLevelItem ( iIdx );
            bool             bFilterFound     = false;

            // DEFINITION: if "#" is set at the beginning of the filter text, we show
            //             occupied servers (#397)
            if ( ( sFilterText.indexOf ( "#" ) == 0 ) && ( sFilterText.length() == 1 ) )
            {
                // special case: filter for occupied servers
                if ( pCurListViewItem->childCount() > 0 )
                {
                    bFilterFound = true;
                }
            }
            else
            {
                // search server name
                if ( pCurListViewItem->text ( 0 ).indexOf ( sFilterText, 0, Qt::CaseInsensitive ) >= 0 )
                {
                    bFilterFound = true;
                }

                // search location
                if ( pCurListViewItem->text ( 3 ).indexOf ( sFilterText, 0, Qt::CaseInsensitive ) >= 0 )
                {
                    bFilterFound = true;
                }

                // search children
                for ( int iCCnt = 0; iCCnt < pCurListViewItem->childCount(); iCCnt++ )
                {
                    if ( pCurListViewItem->child ( iCCnt )->text ( 0 ).indexOf ( sFilterText, 0, Qt::CaseInsensitive ) >= 0 )
                    {
                        bFilterFound = true;
                    }
                }
            }

            // only update Hide state if ping time was received
            if ( !pCurListViewItem->text ( 1 ).isEmpty() || bShowCompleteRegList )
            {
                // only update hide and expand status if the hide state has to be changed to
                // preserve if user clicked on expand icon manually
                if ( ( pCurListViewItem->isHidden() && bFilterFound ) || ( !pCurListViewItem->isHidden() && !bFilterFound ) )
                {
                    pCurListViewItem->setHidden ( !bFilterFound );
                    pCurListViewItem->setExpanded ( bShowAllMusicians );
                }
            }
        }
    }
    else
    {
        // if the filter was active but is now disabled, we have to update all list
        // view items for the "ping received" hide state
        if ( bListFilterWasActive )
        {
            const int iServerListLen = lvwServers->topLevelItemCount();

            for ( int iIdx = 0; iIdx < iServerListLen; iIdx++ )
            {
                QTreeWidgetItem* pCurListViewItem = lvwServers->topLevelItem ( iIdx );

                // if ping time is empty, hide item (if not already hidden)
                if ( pCurListViewItem->text ( 1 ).isEmpty() && !bShowCompleteRegList )
                {
                    pCurListViewItem->setHidden ( true );
                }
                else
                {
                    // in case it was hidden, show it and take care of expand
                    if ( pCurListViewItem->isHidden() )
                    {
                        pCurListViewItem->setHidden ( false );
                        pCurListViewItem->setExpanded ( bShowAllMusicians );
                    }
                }
            }

            bListFilterWasActive = false;
        }
    }
}

//void CClientDlg::OnConnectClicked()
//{
//    // get the IP address to be used according to the following definitions:
//    // - if the list has focus and a line is selected, use this line
//    // - if the list has no focus, use the current combo box text
//    QList<QTreeWidgetItem*> CurSelListItemList = lvwServers->selectedItems();

//    if ( CurSelListItemList.count() > 0 )
//    {
//        // get the parent list view item
//        QTreeWidgetItem* pCurSelTopListItem = GetParentListViewItem ( CurSelListItemList[0] );

//        // get host address from selected list view item as a string
//        strSelectedAddress = pCurSelTopListItem->data ( 0, Qt::UserRole ).toString();

//        // store selected server name
//        strSelectedServerName = pCurSelTopListItem->text ( 0 );

//        // set flag that a server list item was chosen to connect
//        bServerListItemWasChosen = true;
//    }
//    else
//    {
//        strSelectedAddress = NetworkUtil::FixAddress ( cbxServerAddr->currentText() );
//    }

//    // tell the parent window that the connection shall be initiated
//    done ( QDialog::Accepted );
//}

void CClientDlg::OnRegionTimerPing()
{
    // send ping messages to the servers in the list
    const int iServerListLen = lvwServers->topLevelItemCount();

    for ( int iIdx = 0; iIdx < iServerListLen; iIdx++ )
    {
        CHostAddress haServerAddress;

        // try to parse host address string which is stored as user data
        // in the server list item GUI control element
        if ( NetworkUtil().ParseNetworkAddress ( lvwServers->topLevelItem ( iIdx )->data ( 0, Qt::UserRole ).toString(),
                                                 haServerAddress,
                                                 bEnableIPv6 ) )
        {
//            qDebug() << "lvwServer listing: " + lvwServers->topLevelItem ( iIdx )->data ( 0, Qt::UserRole ).toString();
            // if address is valid, send ping message using a new thread
            QFuture<void> f = QtConcurrent::run ( &CClientDlg::EmitCLServerListPingMes, this, haServerAddress );
            Q_UNUSED ( f );
        }
    }
}

void CClientDlg::EmitCLServerListPingMes ( const CHostAddress& haServerAddress )
{
    // The ping time messages for all servers should not be sent all in a very
    // short time since it showed that this leads to errors in the ping time
    // measurement (#49). We therefore introduce a short delay for each server
    // (since we are doing this in a separate thread for each server, we do not
    // block the GUI).
    QThread::msleep ( 11 );

    emit CreateCLServerListPingMes ( haServerAddress );
}

void CClientDlg::SetPingTimeAndNumClientsResult ( const CHostAddress& InetAddr, const int iPingTime, const int iNumClients )
{
    // apply the received ping time to the correct server list entry
    QTreeWidgetItem* pCurListViewItem = FindListViewItem ( InetAddr );

    if ( pCurListViewItem )
    {
        // check if this is the first time a ping time is set
        const bool bIsFirstPing = pCurListViewItem->text ( 1 ).isEmpty();
        bool       bDoSorting   = false;

        // update minimum ping time column (invisible, used for sorting) if
        // the new value is smaller than the old value
        int iMinPingTime = pCurListViewItem->text ( 4 ).toInt();

        if ( iMinPingTime > iPingTime )
        {
            // update the minimum ping time with the new lowest value
            iMinPingTime = iPingTime;

            // we pad to a total of 8 characters with zeros to make sure the
            // sorting is done correctly
            pCurListViewItem->setText ( 4, QString ( "%1" ).arg ( iPingTime, 8, 10, QLatin1Char ( '0' ) ) );

            // update the sorting (lowest number on top)
            bDoSorting = true;
        }

        // for debugging it is good to see the current ping time in the list
        // and not the minimum ping time -> overwrite the value for debugging
        if ( bShowCompleteRegList )
        {
            iMinPingTime = iPingTime;
        }

        // Only show minimum ping time in the list since this is the important
        // value. Temporary bad ping measurements are of no interest.
        // Color definition: <= 25 ms green, <= 50 ms yellow, otherwise red
        if ( iMinPingTime <= 25 )
//        if ( iMinPingTime <= 80 )
        {
            pCurListViewItem->setForeground ( 1, Qt::green );
            pCurListViewItem->setText ( 2, "" );
            pCurListViewItem->setText ( 2, "😃" );
        }
        else
        {
            if ( iMinPingTime <= 50 )
//            if ( iMinPingTime <= 100 )
            {
                pCurListViewItem->setForeground ( 1, Qt::yellow );
                pCurListViewItem->setText ( 2, "😑" );
            }
            else
            {
                pCurListViewItem->setForeground ( 1, Qt::red );
                pCurListViewItem->setText ( 2, "😡" );
            }
        }

        // update ping text, take special care if ping time exceeds a
        // certain value
        if ( iMinPingTime > 500 )
        {
            pCurListViewItem->setText ( 1, ">500" );
            pCurListViewItem->setText ( 2, "🤬" );
        }
        else
        {
            // prepend spaces so that we can sort correctly (fieldWidth of
            // 4 is sufficient since the maximum width is ">500") (#201)
            pCurListViewItem->setText ( 1, QString ( "%1" ).arg ( iMinPingTime, 4, 10, QLatin1Char ( ' ' ) ) );
        }

//        // update number of clients text
//        if ( pCurListViewItem->text ( 5 ).toInt() == 0 )
//        {
//            // special case: reduced server list
//            pCurListViewItem->setText ( 2, QString().setNum ( iNumClients ) );
//        }
//        else if ( iNumClients >= pCurListViewItem->text ( 5 ).toInt() )
//        {
//            pCurListViewItem->setText ( 2, QString().setNum ( iNumClients ) + " (full)" );
//        }
//        else
//        {
//            pCurListViewItem->setText ( 2, QString().setNum ( iNumClients ) + "/" + pCurListViewItem->text ( 5 ) );
//        }

        // check if the number of child list items matches the number of
        // connected clients, if not then request the client names
        if ( iNumClients != pCurListViewItem->childCount() )
        {
            emit CreateCLServerListReqConnClientsListMes ( InetAddr );
        }

        // this is the first time a ping time was received, set item to visible
        // UNLESS it is Directory Server, in which case keep it hidden
        if ( bIsFirstPing && !pCurListViewItem->text ( 0 ).contains("Directory"))
        {
            pCurListViewItem->setHidden ( false );
        }

        // Update sorting. Note that the sorting must be the last action for the
        // current item since the topLevelItem(iIdx) is then no longer valid.
        // To avoid that the list is sorted shortly before a double click (which
        // could lead to connecting an incorrect server) the sorting is disabled
        // as long as the mouse is over the list (but it is not disabled for the
        // initial timer of about 2s, see TimerInitialSort) (#293).
//        if ( bDoSorting && !bShowCompleteRegList &&
//             ( TimerInitialSort.isActive() || !lvwServers->underMouse() ) ) // do not sort if "show all servers"
//        {
//            lvwServers->sortByColumn ( 2, Qt::AscendingOrder );
//        }

//        if ( bDoSorting && TimerInitialSort.isActive() ) // do not sort if "show all servers"
//        {
//            lvwServers->sortByColumn ( 1, Qt::AscendingOrder );
//        }
        lvwServers->sortByColumn ( 1, Qt::AscendingOrder );

    }

    // if no server item has children, do not show decoration
    bool      bAnyListItemHasChilds = false;
    const int iServerListLen        = lvwServers->topLevelItemCount();

    for ( int iIdx = 0; iIdx < iServerListLen; iIdx++ )
    {
        // check if the current list item has children
        if ( lvwServers->topLevelItem ( iIdx )->childCount() > 0 )
        {
            bAnyListItemHasChilds = true;
        }
    }

    if ( !bAnyListItemHasChilds )
    {
        lvwServers->setRootIsDecorated ( false );
    }

    // we may have changed the Hidden state for some items, if a filter was active, we now
    // have to update it to void lines appear which do not satisfy the filter criteria
    UpdateListFilter();
}

QTreeWidgetItem* CClientDlg::FindListViewItem ( const CHostAddress& InetAddr )
{
    const int iServerListLen = lvwServers->topLevelItemCount();

    for ( int iIdx = 0; iIdx < iServerListLen; iIdx++ )
    {
        // compare the received address with the user data string of the
        // host address by a string compare
        if ( !lvwServers->topLevelItem ( iIdx )->data ( 0, Qt::UserRole ).toString().compare ( InetAddr.toString() ) )
        {
            return lvwServers->topLevelItem ( iIdx );
        }
    }

    return nullptr;
}

QTreeWidgetItem* CClientDlg::GetParentListViewItem ( QTreeWidgetItem* pItem )
{
    // check if the current item is already the top item, i.e. the parent
    // query fails and returns null
    if ( pItem->parent() )
    {
        // we only have maximum one level, i.e. if we call the parent function
        // we are at the top item
        return pItem->parent();
    }
    else
    {
        // this item is already the top item
        return pItem;
    }
}

void CClientDlg::DeleteAllListViewItemChilds ( QTreeWidgetItem* pItem )
{
    // loop over all children
    while ( pItem->childCount() > 0 )
    {
        // get the first child in the list
        QTreeWidgetItem* pCurChildItem = pItem->child ( 0 );

        // remove it from the item (note that the object is not deleted)
        pItem->removeChild ( pCurChildItem );

        // delete the object to avoid a memory leak
        delete pCurChildItem;
    }
}

void CClientDlg::UpdateDirectoryServerComboBox()
{
    // directory type combo box
    cbxDirectoryServer->clear();
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_DEFAULT ) );
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_ANY_GENRE2 ) );
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_ANY_GENRE3 ) );
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_GENRE_ROCK ) );
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_GENRE_JAZZ ) );
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_GENRE_CLASSICAL_FOLK ) );
    cbxDirectoryServer->addItem ( DirectoryTypeToString ( AT_GENRE_CHORAL ) );

    // because custom directories are always added to the top of the vector, add the vector
    // contents to the combobox in reverse order
    for ( int i = MAX_NUM_SERVER_ADDR_ITEMS - 1; i >= 0; i-- )
    {
        if ( pSettings->vstrDirectoryAddress[i] != "" )
        {
            // add vector index (i) to the combobox as user data
            cbxDirectoryServer->addItem ( pSettings->vstrDirectoryAddress[i], i );
        }
    }
}

// #if defined ( Q_OS_WINDOWS )
// // for kdasio_builtin stuff
// void CClientDlg::kdasio_setup() {
//     // init mmcpl proc
//     mmcplProc = nullptr;
//     // init our singleton QMediaDevices object
//     m_devices = new QMediaDevices();

//     // set up signals
//     connect(sharedPushButton, &QPushButton::clicked, this, &CClientDlg::sharedModeSet);
//     connect(exclusivePushButton, &QPushButton::clicked, this, &CClientDlg::exclusiveModeSet);
//     connect(inputDeviceBox, QOverload<int>::of(&QComboBox::activated), this, &CClientDlg::inputDeviceChanged);
//     connect(outputDeviceBox, QOverload<int>::of(&QComboBox::activated), this, &CClientDlg::outputDeviceChanged);
// //    connect(inputAudioSettButton, &QPushButton::pressed, this, &CClientDlg::inputAudioSettClicked);
// //    connect(outputAudioSettButton, &QPushButton::pressed, this, &CClientDlg::outputAudioSettClicked);
//     connect(bufferSizeSlider, &QSlider::valueChanged, this, &CClientDlg::bufferSizeChanged);
//     connect(bufferSizeSlider, &QSlider::valueChanged, this, &CClientDlg::bufferSizeDisplayChange);
//     // for device refresh
//     connect(m_devices, &QMediaDevices::audioInputsChanged, this, &CClientDlg::updateInputsList);
//     connect(m_devices, &QMediaDevices::audioOutputsChanged, this, &CClientDlg::updateOutputsList);
//     // for win ctrl panel
//     connect(winCtrlPanelButton, &QPushButton::clicked, this, &CClientDlg::openWinCtrlPanel);

//     // populate input device choices
//     inputDeviceBox->clear();
//     const auto input_devices = m_devices->audioInputs();
//     for (auto &deviceInfo: input_devices)
//         inputDeviceBox->addItem(deviceInfo.description(), QVariant::fromValue(deviceInfo));

//     // populate output device choices
//     outputDeviceBox->clear();
//     const auto output_devices = m_devices->audioOutputs();
//     for (auto &deviceInfo: output_devices)
//         outputDeviceBox->addItem(deviceInfo.description(), QVariant::fromValue(deviceInfo));


//     // parse .kdasio_builtin.toml
//     // parse .KoordASIO.toml
//     // FIXME - doesn't actually test that the selected devices are correct with current device list
//     std::ifstream ifs;
//     ifs.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
//     try {
//         ifs.open(kdasio_config_path.toStdString(), std::ifstream::in);
//         toml::ParseResult pr = toml::parse(ifs);
//         qDebug("Attempted to parse toml file...");
//         ifs.close();
//         if (!pr.valid()) {
//             setKdasio_builtinDefaults();
//         } else {
//             setValuesFromToml(&ifs, &pr);
//         }
//     }
//     catch (std::ifstream::failure e) {
//         qDebug("Failed to open file ...");
//         setKdasio_builtinDefaults();
//     }

// }

// void CClientDlg::updateInputsList() {
//     inputDeviceBox->clear();
//     const auto input_devices = m_devices->audioInputs();
//     for (auto &deviceInfo: input_devices)
//         inputDeviceBox->addItem(deviceInfo.description(), QVariant::fromValue(deviceInfo));
//     // write the new resultant config
//     inputDeviceName = inputDeviceBox->currentText();
//     writeTomlFile();
// //    OnSoundcardReactivate();
// }

// void CClientDlg::updateOutputsList() {
//     // populate output device choices
//     outputDeviceBox->clear();
//     const auto output_devices = m_devices->audioOutputs();
//     for (auto &deviceInfo: output_devices)
//         outputDeviceBox->addItem(deviceInfo.description(), QVariant::fromValue(deviceInfo));
//     qInfo() << "Updating Outputs ...";
//     // write the new resultant config
//     outputDeviceName = outputDeviceBox->currentText();
//     writeTomlFile();
// //    OnSoundcardReactivate();
// }

// void CClientDlg::setValuesFromToml(std::ifstream *ifs, toml::ParseResult *pr)
// {
//     qInfo("Parsed a valid TOML file.");
//     // only recognise our accepted INPUT values - the others are hardcoded
//     const toml::Value& v = pr->value;
//     // get bufferSize
//     const toml::Value* bss = v.find("bufferSizeSamples");
//     if (bss && bss->is<int>()) {
//         if (bss->as<int>() == 32||64||128||256||512||1024||2048) {
//             bufferSize = bss->as<int>();
//         } else {
//             bufferSize = 64;
//         }
//         // update UI
//         bufferSizeSlider->setValue(bufferSizes.indexOf(bufferSize));
//         bufferSizeDisplay->display(bufferSize);
//         // update conf
//         bufferSizeChanged(bufferSizes.indexOf(bufferSize));
//     }
//     // get input stream stuff
//     const toml::Value* input_dev = v.find("input.device");
//     if (input_dev && input_dev->is<std::string>()) {
//         // if setCurrentText fails some sensible choice is made
//         inputDeviceBox->setCurrentText(QString::fromStdString(input_dev->as<std::string>()));
//         inputDeviceChanged(inputDeviceBox->currentIndex());
//     } else {
//         inputDeviceBox->setCurrentText("Default Input Device");
//         inputDeviceChanged(inputDeviceBox->currentIndex());
//     }
//     const toml::Value* input_excl = v.find("input.wasapiExclusiveMode");
//     if (input_excl && input_excl->is<bool>()) {
//         exclusive_mode = input_excl->as<bool>();
//     } else {
//         exclusive_mode = false;
//     }
//     // get output stream stuff
//     const toml::Value* output_dev = v.find("output.device");
//     if (output_dev && output_dev->is<std::string>()) {
//         // if setCurrentText fails some sensible choice is made
//         outputDeviceBox->setCurrentText(QString::fromStdString(output_dev->as<std::string>()));
//         outputDeviceChanged(outputDeviceBox->currentIndex());
//     } else {
//         outputDeviceBox->setCurrentText("Default Output Device");
//         outputDeviceChanged(outputDeviceBox->currentIndex());
//     }
//     const toml::Value* output_excl = v.find("output.wasapiExclusiveMode");
//     if (output_excl && output_excl->is<bool>()) {
//         exclusive_mode = output_excl->as<bool>();
//     } else {
//         exclusive_mode = false;
//     }
//     setOperationMode();

// }

// void CClientDlg::setKdasio_builtinDefaults()
// {
//     // set defaults
//     qInfo("Setting KdASIO defaults");
//     bufferSize = 32;
//     exclusive_mode = true;
//     // find system audio device defaults
//     QAudioDevice inputInfo(QMediaDevices::defaultAudioInput());
//     inputDeviceName = inputInfo.description();
//     QAudioDevice outputInfo(QMediaDevices::defaultAudioOutput());
//     outputDeviceName = outputInfo.description();
//     // set stuff - up to 4 file updates in quick succession - FIXME
//     bufferSizeSlider->setValue(bufferSizes.indexOf(bufferSize));
//     bufferSizeDisplay->display(bufferSize);
//     bufferSizeChanged(bufferSizes.indexOf(bufferSize));
//     inputDeviceBox->setCurrentText(inputDeviceName);
//     inputDeviceChanged(inputDeviceBox->currentIndex());
//     outputDeviceBox->setCurrentText(outputDeviceName);
//     outputDeviceChanged(outputDeviceBox->currentIndex());
//     setOperationMode();
// }


// void CClientDlg::writeTomlFile()
// {
//     // REF: https://github.com/dechamps/FlexASIO/blob/master/CONFIGURATION.md
//     // Write MINIMAL config to .kdasio_builtin.toml, like this:
//     /*
//         backend = "Windows WASAPI"
//         bufferSizeSamples = bufferSize

//         [input]
//         device=inputDevice
//         suggestedLatencySeconds = 0.0
//         wasapiExclusiveMode = inputExclusiveMode

//         [output]
//         device=outputDevice
//         suggestedLatencySeconds = 0.0
//         wasapiExclusiveMode = outputExclusiveMode
//     */
//     qInfo() << "writeTomlFile(): Opening file for writing: " << kdasio_config_path;
//     qInfo() << "writeTomlFile(): input device: " << inputDeviceName;
//     qInfo() << "writeTomlFile(): output device: " << outputDeviceName;
//     QFile file(kdasio_config_path);
//     if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//         return;
//     QTextStream out(&file);
//     // need to explicitly set UTF-8 for non-ASCII character support
//     out.setEncoding(QStringConverter::Utf8);
//     // out.setCodec("UTF-8");

//     //FIXME should really write to intermediate buffer, THEN to file - to make single write on file
//     // is this a single write action ???
//     out << "backend = \"Windows WASAPI\"" << "\n"
//         << "bufferSizeSamples = " << bufferSize << "\n"
//         << "\n"
//         << "[input]" << "\n"
//         << "device = \"" << inputDeviceName << "\"\n"
//         << "suggestedLatencySeconds = 0.0" << "\n"
//         << "wasapiExclusiveMode = " << (exclusive_mode ? "true" : "false") << "\n"
//         << "\n"
//         << "[output]" << "\n"
//         << "device = \"" << outputDeviceName << "\"\n"
//         << "suggestedLatencySeconds = 0.0" << "\n"
//         << "wasapiExclusiveMode = " << (exclusive_mode ? "true" : "false") << "\n";
//     qInfo("Just wrote toml file...");

//     // force driver settings refresh
// //    OnSoundcardReactivate();
// }

// void CClientDlg::bufferSizeChanged(int idx)
// {
//     qInfo() << "bufferSizeChanged()";
//     // select from 32 , 64, 128, 256, 512, 1024, 2048
//     // This a) gives a nice easy UI rather than choosing your own integer
//     // AND b) makes it easier to do a live-refresh of the toml file,
//     // THUS avoiding lots of spurious intermediate updates on buffer changes
//     bufferSize = bufferSizes[idx];
//     bufferSizeSlider->setValue(idx);
//     // Don't do any latency calculation for now, it is misleading as doesn't account for much of the whole audio chain
// //    latencyLabel->setText(QString::number(double(bufferSize) / 48, 'f', 2));
//     writeTomlFile();
//     OnSoundcardReactivate();
// }

// void CClientDlg::bufferSizeDisplayChange(int idx)
// {
//     qInfo() << "bufferSizeDisplayChange()";
//     bufferSize = bufferSizes[idx];
//     bufferSizeDisplay->display(bufferSize);
// }

// void CClientDlg::setOperationMode()
// {
//     if (exclusive_mode) {
//         exclusiveModeSet();
//     }
//     else {
//         sharedModeSet();
//     }
// }

// void CClientDlg::sharedModeSet()
// {
//     sharedPushButton->setChecked(true);
//     exclusive_mode = false;
//     writeTomlFile();
//     OnSoundcardReactivate();
// }

// void CClientDlg::exclusiveModeSet()
// {
//     exclusivePushButton->setChecked(true);
//     exclusive_mode = true;
//     writeTomlFile();
//     OnSoundcardReactivate();
// }

// void CClientDlg::inputDeviceChanged(int idx)
// {
//     qInfo() << "inputDeviceChanged()";
//     if (inputDeviceBox->count() == 0)
//         return;
//     // device has changed
//     m_inputDeviceInfo = inputDeviceBox->itemData(idx).value<QAudioDevice>();
//     inputDeviceName = m_inputDeviceInfo.description();
//     writeTomlFile();
//     OnSoundcardReactivate();
// }

// void CClientDlg::outputDeviceChanged(int idx)
// {
//     qInfo() << "outputDeviceChanged()";
//     if (outputDeviceBox->count() == 0)
//         return;
//     // device has changed
//     m_outputDeviceInfo = outputDeviceBox->itemData(idx).value<QAudioDevice>();
//     outputDeviceName = m_outputDeviceInfo.description();
//     writeTomlFile();
//     OnSoundcardReactivate();
// }

// //void CClientDlg::inputAudioSettClicked()
// //{
// //    // open Windows audio input settings control panel
// //    //FIXME - this process control does NOT work as Windows forks+kills the started process immediately? or something
// //    if (mmcplProc != nullptr) {
// //        mmcplProc->kill();
// //    }
// //    mmcplProc = new QProcess(this);
// //    mmcplProc->start("control", QStringList() << inputAudioSettPath);
// //}

// //void CClientDlg::outputAudioSettClicked()
// //{
// //    // open Windows audio output settings control panel
// //    //FIXME - this process control does NOT work as Windows forks+kills the started process immediately? or something
// //    if (mmcplProc != nullptr) {
// //        mmcplProc->kill();
// //    }
// //    mmcplProc = new QProcess(this);
// //    mmcplProc->start("control", QStringList() << outputAudioSettPath);
// //}

// void CClientDlg::openWinCtrlPanel()
// {
//     QDesktopServices::openUrl(QUrl("ms-settings:sound-devices", QUrl::TolerantMode));
// }
// #endif
// // end Windows-only built-in asio config stuff

