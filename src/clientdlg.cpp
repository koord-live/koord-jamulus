/******************************************************************************\
 * Copyright (c) 2004-2022
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
#include <QtQuickWidgets>

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
    bBasicConnectDlgWasShown ( false ),
    bMIDICtrlUsed ( !strMIDISetup.isEmpty() ),
    bDetectFeedback ( false ),
    bEnableIPv6 ( bNEnableIPv6 ),
    eLastRecorderState ( RS_UNDEFINED ), // for SetMixerBoardDeco
    eLastDesign ( GD_ORIGINAL ),         //          "
    strSelectedAddress (""),
    BasicConnectDlg ( pNSetP, parent ),
    AnalyzerConsole ( pNCliP, parent )
{
    //FIXME cruft to remove later - just remove compiler warnings for now
    if (bNewShowComplRegConnList == false)
        ;
    if (bShowAnalyzerConsole == false)
        ;
    // end cruft

    //FIXME - possibly not necessary
#if defined(ANDROID)
    setCentralWidget(backgroundFrame);
#endif

    // setup main UI
    setupUi ( this );

    // set up net manager for https requests
    qNam = new QNetworkAccessManager;

// FIXME - exception for Android
// - QuickView does NOT work as for other OS - when setSource is changed from "nosession" to webview, view goes full-screen
#if defined(ANDROID)
    quickWidget = new QQuickWidget;
    // need SizeRootObjectToView for QuickWidget, otherwise web content doesn't load ??
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl("qrc:/androidwebview.qml"));
    videoTab->layout()->addWidget(quickWidget);
    QQmlContext* context = quickWidget->rootContext();
#else
// Note: use QuickView to workaround problems with QuickWidget
    quickView = new QQuickView();
    QWidget *container = QWidget::createWindowContainer(quickView, this);
    quickView->setSource(QUrl("qrc:/nosessionview.qml"));
    videoTab->layout()->addWidget(container);
    QQmlContext* context = quickView->rootContext();
#endif

    //FIXME prob don't want whole ClientDlg object ?
    context->setContextProperty("_clientdlg", this);

    // initialize video_url with blank value to start
    strVideoUrl = "";

    // Set up touch on all widgets' viewports which inherit from QAbstractScrollArea
    // https://doc.qt.io/qt-6/qtouchevent.html#details
    scrollArea->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    txvHelp->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    txvAbout->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    txvChatWindow->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    QScroller::grabGesture(scrollArea, QScroller::TouchGesture);
    QScroller::grabGesture(txvHelp, QScroller::TouchGesture);
    QScroller::grabGesture(txvAbout, QScroller::TouchGesture);
    QScroller::grabGesture(txvChatWindow, QScroller::TouchGesture);

    // add Session Chat stuff
    // input message text
    edtLocalInputText->setWhatsThis ( "<b>" + tr ( "Input Message Text" ) + ":</b> " +
                                      tr ( "Chat here while a session is active." ) );

    // clear chat window and edit line
    txvChatWindow->clear();
    edtLocalInputText->clear();

    // we do not want to show a cursor in the chat history
    txvChatWindow->setCursorWidth ( 0 );

    // set a placeholder text to make sure where to type the message in (#384)
    edtLocalInputText->setPlaceholderText ( tr ( "Type a message here" ) );
    // disable chat widgets
    butSend->setEnabled(false);
    edtLocalInputText->setEnabled(false);

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
                                "you will not have much fun using the application." ) +
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

    ledBuffers->setAccessibleName ( tr ( "Local Jitter Buffer status LED indicator" ) );

    // current connection status parameter
    QString strConnStats = "<b>" +
                           tr ( "Current Connection Status "
                                "Parameter" ) +
                           ":</b> " +
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
    ledDelay->setWhatsThis ( strConnStats );
    ledDelay->setToolTip ( tr ( "If this LED indicator turns red, "
                                "you will not have much fun using "
                                "the %1 software." )
                               .arg ( APP_NAME ) +
                           TOOLTIP_COM_END_TEXT );
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
    butConnect->setText ( tr ( "Join..." ) );

    // don't show link info to start with
//    linkField->setVisible(false);

    // init new session button text
//    butNewStart->setText ( tr ( "&New Session" ) );
    // FIXME DON'T hardcode here!
//    butNewStart->setText ( tr ( "Create" ) );

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
//    sldAudioReverb->setTickInterval ( AUD_REVERB_MAX / 5 );

    // init input boost
    pClient->SetInputBoost ( pSettings->iInputBoost );

    // init reverb channel
    UpdateRevSelection();

    // init connect dialog
//    ConnectDlg.SetShowAllMusicians ( pSettings->bConnectDlgShowAllMusicians );

    // set window title (with no clients connected -> "0")
    SetMyWindowTitle ( 0 );

    // prepare Mute Myself info label (invisible by default)
//    lblGlobalInfoLabel->setStyleSheet ( ".QLabel { background: red; }" );
//    lblGlobalInfoLabel->hide();

    // prepare update check info label (invisible by default)
//    lblUpdateCheck->setOpenExternalLinks ( true ); // enables opening a web browser if one clicks on a html link
//    lblUpdateCheck->setText ( "<font color=\"red\"><b>" + APP_UPGRADE_AVAILABLE_MSG_TEXT.arg ( APP_NAME ).arg ( VERSION ) + "</b></font>" );
//    lblUpdateCheck->hide();

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

//    // File menu  --------------------------------------------------------------
//    QMenu* pFileMenu = new QMenu ( tr ( "&File" ), this );

//    pFileMenu->addAction ( tr ( "&Connection Setup..." ), this, SLOT ( OnOpenConnectionSetupDialog() ), QKeySequence ( Qt::CTRL + Qt::Key_C ) );

//    pFileMenu->addSeparator();

//    pFileMenu->addAction ( tr ( "&Load Mixer Channels Setup..." ), this, SLOT ( OnLoadChannelSetup() ) );

//    pFileMenu->addAction ( tr ( "&Save Mixer Channels Setup..." ), this, SLOT ( OnSaveChannelSetup() ) );

//    pFileMenu->addSeparator();

//    pFileMenu->addAction ( tr ( "E&xit" ), this, SLOT ( close() ), QKeySequence ( Qt::CTRL + Qt::Key_Q ) );

//    // Edit menu  --------------------------------------------------------------
//    QMenu* pEditMenu = new QMenu ( tr ( "&Edit" ), this );

//    pEditMenu->addAction ( tr ( "Clear &All Stored Solo and Mute Settings" ), this, SLOT ( OnClearAllStoredSoloMuteSettings() ) );

//    pEditMenu->addAction ( tr ( "Set All Faders to New Client &Level" ),
//                           this,
//                           SLOT ( OnSetAllFadersToNewClientLevel() ),
//                           QKeySequence ( Qt::CTRL + Qt::Key_L ) );

//    pEditMenu->addAction ( tr ( "Auto-Adjust all &Faders" ), this, SLOT ( OnAutoAdjustAllFaderLevels() ), QKeySequence ( Qt::CTRL + Qt::Key_F ) );

//    // View menu  --------------------------------------------------------------
//    QMenu* pViewMenu = new QMenu ( tr ( "&View" ), this );

//    // own fader first option: works from server version 3.5.5 which supports sending client ID back to client
//    QAction* OwnFaderFirstAction =
//        pViewMenu->addAction ( tr ( "O&wn Fader First" ), this, SLOT ( OnOwnFaderFirst() ), QKeySequence ( Qt::CTRL + Qt::Key_W ) );

//    pViewMenu->addSeparator();

//    QAction* NoSortAction =
//        pViewMenu->addAction ( tr ( "N&o User Sorting" ), this, SLOT ( OnNoSortChannels() ), QKeySequence ( Qt::CTRL + Qt::Key_O ) );

//    QAction* ByNameAction =
//        pViewMenu->addAction ( tr ( "Sort Users by &Name" ), this, SLOT ( OnSortChannelsByName() ), QKeySequence ( Qt::CTRL + Qt::Key_N ) );

//    QAction* ByInstrAction = pViewMenu->addAction ( tr ( "Sort Users by &Instrument" ),
//                                                    this,
//                                                    SLOT ( OnSortChannelsByInstrument() ),
//                                                    QKeySequence ( Qt::CTRL + Qt::Key_I ) );

//    QAction* ByGroupAction =
//        pViewMenu->addAction ( tr ( "Sort Users by &Group" ), this, SLOT ( OnSortChannelsByGroupID() ), QKeySequence ( Qt::CTRL + Qt::Key_G ) );

//    QAction* ByCityAction =
//        pViewMenu->addAction ( tr ( "Sort Users by &City" ), this, SLOT ( OnSortChannelsByCity() ), QKeySequence ( Qt::CTRL + Qt::Key_T ) );

//    OwnFaderFirstAction->setCheckable ( true );
//    OwnFaderFirstAction->setChecked ( pSettings->bOwnFaderFirst );

//    // the sorting menu entries shall be checkable and exclusive
//    QActionGroup* SortActionGroup = new QActionGroup ( this );
//    SortActionGroup->setExclusive ( true );
//    NoSortAction->setCheckable ( true );
//    SortActionGroup->addAction ( NoSortAction );
//    ByNameAction->setCheckable ( true );
//    SortActionGroup->addAction ( ByNameAction );
//    ByInstrAction->setCheckable ( true );
//    SortActionGroup->addAction ( ByInstrAction );
//    ByGroupAction->setCheckable ( true );
//    SortActionGroup->addAction ( ByGroupAction );
//    ByCityAction->setCheckable ( true );
//    SortActionGroup->addAction ( ByCityAction );

//    // initialize sort type setting (i.e., recover stored setting)
//    switch ( pSettings->eChannelSortType )
//    {
//    case ST_NO_SORT:
//        NoSortAction->setChecked ( true );
//        break;
//    case ST_BY_NAME:
//        ByNameAction->setChecked ( true );
//        break;
//    case ST_BY_INSTRUMENT:
//        ByInstrAction->setChecked ( true );
//        break;
//    case ST_BY_GROUPID:
//        ByGroupAction->setChecked ( true );
//        break;
//    case ST_BY_CITY:
//        ByCityAction->setChecked ( true );
//        break;
//    }
//    MainMixerBoard->SetFaderSorting ( pSettings->eChannelSortType );

//    pViewMenu->addSeparator();

//    pViewMenu->addAction ( tr ( "C&hat..." ), this, SLOT ( OnOpenChatDialog() ), QKeySequence ( Qt::CTRL + Qt::Key_H ) );

//    // optionally show analyzer console entry
//    if ( bShowAnalyzerConsole )
//    {
//        pViewMenu->addAction ( tr ( "&Analyzer Console..." ), this, SLOT ( OnOpenAnalyzerConsole() ) );
//    }

//    pViewMenu->addSeparator();

//    // Settings menu  --------------------------------------------------------------
//    QMenu* pSettingsMenu = new QMenu ( tr ( "Sett&ings" ), this );

//    pSettingsMenu->addAction ( tr ( "My &Profile..." ), this, SLOT ( OnOpenUserProfileSettings() ), QKeySequence ( Qt::CTRL + Qt::Key_P ) );

//    pSettingsMenu->addAction ( tr ( "Audio/Network &Settings..." ), this, SLOT ( OnOpenAudioNetSettings() ), QKeySequence ( Qt::CTRL + Qt::Key_S ) );

//    pSettingsMenu->addAction ( tr ( "A&dvanced Settings..." ), this, SLOT ( OnOpenAdvancedSettings() ), QKeySequence ( Qt::CTRL + Qt::Key_D ) );

//    // Main menu bar -----------------------------------------------------------
//    QMenuBar* pMenu = new QMenuBar ( this );

//    pMenu->addMenu ( pFileMenu );
//    pMenu->addMenu ( pEditMenu );
//    pMenu->addMenu ( pViewMenu );
//    pMenu->addMenu ( pSettingsMenu );
//    pMenu->addMenu ( new CHelpMenu ( true, this ) );

//    // Now tell the layout about the menu
//    layout()->setMenuBar ( pMenu );

    // Window positions --------------------------------------------------------
    // main window
    if ( !pSettings->vecWindowPosMain.isEmpty() && !pSettings->vecWindowPosMain.isNull() )
    {
        restoreGeometry ( pSettings->vecWindowPosMain );
    }

//    // settings window
//    if ( !pSettings->vecWindowPosSettings.isEmpty() && !pSettings->vecWindowPosSettings.isNull() )
//    {
//        ClientSettingsDlg.restoreGeometry ( pSettings->vecWindowPosSettings );
//    }

//    if ( pSettings->bWindowWasShownSettings )
//    {
//        ShowGeneralSettings ( pSettings->iSettingsTab );
//    }

//    // chat window
//    if ( !pSettings->vecWindowPosChat.isEmpty() && !pSettings->vecWindowPosChat.isNull() )
//    {
//        ChatDlg.restoreGeometry ( pSettings->vecWindowPosChat );
//    }

//    if ( pSettings->bWindowWasShownChat )
//    {
//        ShowChatWindow();
//    }

//    // connection setup window
//    if ( !pSettings->vecWindowPosConnect.isEmpty() && !pSettings->vecWindowPosConnect.isNull() )
//    {
//        ConnectDlg.restoreGeometry ( pSettings->vecWindowPosConnect );
//    }

    // basic connection setup window
    if ( !pSettings->vecWindowPosBasicConnect.isEmpty() && !pSettings->vecWindowPosBasicConnect.isNull() )
    {
        BasicConnectDlg.restoreGeometry ( pSettings->vecWindowPosBasicConnect );
    }

// START OF SETTINGS STUFF ===========================================================

    // Add help text to controls -----------------------------------------------
    // local audio input fader
    QString strAudFader = "<b>" + tr ( "Local Audio Input Fader" ) + ":</b> " +
                          tr ( "Controls the relative levels of the left and right local audio "
                               "channels. For a mono signal it acts as a pan between the two channels. "
                               "For example, if a microphone is connected to "
                               "the right input channel and an instrument is connected to the left "
                               "input channel which is much louder than the microphone, move the "
                               "audio fader in a direction where the label above the fader shows "
                               "%1, where %2 is the current attenuation indicator." )
                              .arg ( "<i>" + tr ( "L" ) + " -x</i>", "<i>x</i>" );

    lblAudioPan->setWhatsThis ( strAudFader );
    lblAudioPanValue->setWhatsThis ( strAudFader );
//    sldAudioPan->setWhatsThis ( strAudFader );

//    sldAudioPan->setAccessibleName ( tr ( "Local audio input fader (left/right)" ) );

    // jitter buffer
    QString strJitterBufferSize = "<b>" + tr ( "Jitter Buffer Size" ) + ":</b> " +
                                  tr ( "The jitter buffer compensates for network and sound card timing jitters. The "
                                       "size of the buffer therefore influences the quality of "
                                       "the audio stream (how many dropouts occur) and the overall delay "
                                       "(the longer the buffer, the higher the delay)." ) +
                                  "<br>" +
                                  tr ( "You can set the jitter buffer size manually for the local client "
                                       "and the remote server. For the local jitter buffer, dropouts in the "
                                       "audio stream are indicated by the light below the "
                                       "jitter buffer size faders. If the light turns to red, a buffer "
                                       "overrun/underrun has taken place and the audio stream is interrupted." ) +
                                  "<br>" +
                                  tr ( "The jitter buffer setting is therefore a trade-off between audio "
                                       "quality and overall delay." ) +
                                  "<br>" +
                                  tr ( "If the Auto setting is enabled, the jitter buffers of the local client and "
                                       "the remote server are set automatically "
                                       "based on measurements of the network and sound card timing jitter. If "
                                       "Auto is enabled, the jitter buffer size faders are "
                                       "disabled (they cannot be moved with the mouse)." );

    QString strJitterBufferSizeTT = tr ( "If the Auto setting "
                                         "is enabled, the network buffers of the local client and "
                                         "the remote server are set to a conservative "
                                         "value to minimize the audio dropout probability. To tweak the "
                                         "audio delay/latency it is recommended to disable the Auto setting "
                                         "and to lower the jitter buffer size manually by "
                                         "using the sliders until your personal acceptable amount "
                                         "of dropouts is reached. The LED indicator will display the audio "
                                         "dropouts of the local jitter buffer with a red light." ) +
                                    TOOLTIP_COM_END_TEXT;

    lblNetBuf->setWhatsThis ( strJitterBufferSize );
    lblNetBuf->setToolTip ( strJitterBufferSizeTT );
    grbJitterBuffer->setWhatsThis ( strJitterBufferSize );
    grbJitterBuffer->setToolTip ( strJitterBufferSizeTT );
    sldNetBuf->setWhatsThis ( strJitterBufferSize );
    sldNetBuf->setAccessibleName ( tr ( "Local jitter buffer slider control" ) );
    sldNetBuf->setToolTip ( strJitterBufferSizeTT );
    sldNetBufServer->setWhatsThis ( strJitterBufferSize );
    sldNetBufServer->setAccessibleName ( tr ( "Server jitter buffer slider control" ) );
    sldNetBufServer->setToolTip ( strJitterBufferSizeTT );
    chbAutoJitBuf->setAccessibleName ( tr ( "Auto jitter buffer switch" ) );
    chbAutoJitBuf->setToolTip ( strJitterBufferSizeTT );

#if !defined( WITH_JACK )
    // sound card device
    lblSoundcardDevice->setWhatsThis ( "<b>" + tr ( "Audio Device" ) + ":</b> " +
                                       tr ( "Under the Windows operating system the ASIO driver (sound card) can be "
                                            "selected using %1. If the selected ASIO driver is not valid an error "
                                            "message is shown and the previous valid driver is selected. "
                                            "Under macOS the input and output hardware can be selected." )
                                           .arg ( APP_NAME ) +
                                       "<br>" +
                                       tr ( "If the driver is selected during an active connection, the connection "
                                            "is stopped, the driver is changed and the connection is started again "
                                            "automatically." ) );

    cbxSoundcard->setAccessibleName ( tr ( "Sound card device selector combo box" ) );

#    if defined( _WIN32 )
    // set Windows specific tool tip
    cbxSoundcard->setToolTip ( tr ( "If the ASIO4ALL driver is used, "
                                    "please note that this driver usually introduces approx. 10-30 ms of "
                                    "additional audio delay. Using a sound card with a native ASIO driver "
                                    "is therefore recommended." ) +
                               "<br>" +
                               tr ( "If you are using the kX ASIO "
                                    "driver, make sure to connect the ASIO inputs in the kX DSP settings "
                                    "panel." ) +
                               TOOLTIP_COM_END_TEXT );
#    endif

    // sound card input/output channel mapping
    QString strSndCrdChanMapp = "<b>" + tr ( "Sound Card Channel Mapping" ) + ":</b> " +
                                tr ( "If the selected sound card device offers more than one "
                                     "input or output channel, the Input Channel Mapping and Output "
                                     "Channel Mapping settings are visible." ) +
                                "<br>" +
                                tr ( "For each %1 input/output channel (left and "
                                     "right channel) a different actual sound card channel can be "
                                     "selected." )
                                    .arg ( APP_NAME );

    lblInChannelMapping->setWhatsThis ( strSndCrdChanMapp );
    lblOutChannelMapping->setWhatsThis ( strSndCrdChanMapp );
    cbxLInChan->setWhatsThis ( strSndCrdChanMapp );
    cbxLInChan->setAccessibleName ( tr ( "Left input channel selection combo box" ) );
    cbxRInChan->setWhatsThis ( strSndCrdChanMapp );
    cbxRInChan->setAccessibleName ( tr ( "Right input channel selection combo box" ) );
    cbxLOutChan->setWhatsThis ( strSndCrdChanMapp );
    cbxLOutChan->setAccessibleName ( tr ( "Left output channel selection combo box" ) );
    cbxROutChan->setWhatsThis ( strSndCrdChanMapp );
    cbxROutChan->setAccessibleName ( tr ( "Right output channel selection combo box" ) );
#endif

    // enable OPUS64
    chbEnableOPUS64->setWhatsThis ( "<b>" + tr ( "Enable Small Network Buffers" ) + ":</b> " +
                                    tr ( "Enables support for very small network audio packets. These "
                                         "network packets are only actually used if the sound card buffer delay is smaller than %1 samples. The "
                                         "smaller the network buffers, the lower the audio latency. But at the same time "
                                         "the network load and the probability of audio dropouts or sound artifacts increases." )
                                        .arg ( DOUBLE_SYSTEM_FRAME_SIZE_SAMPLES ) );

    chbEnableOPUS64->setAccessibleName ( tr ( "Enable small network buffers check box" ) );

    // sound card buffer delay
    QString strSndCrdBufDelay = "<b>" + tr ( "Sound Card Buffer Delay" ) + ":</b> " +
                                tr ( "The buffer delay setting is a fundamental setting of %1. "
                                     "This setting has an influence on many connection properties." )
                                    .arg ( APP_NAME ) +
                                "<br>" + tr ( "Three buffer sizes can be selected" ) +
                                ":<ul>"
                                "<li>" +
                                tr ( "64 samples: Provides the lowest latency but does not work with all sound cards." ) +
                                "</li>"
                                "<li>" +
                                tr ( "128 samples: Should work for most available sound cards." ) +
                                "</li>"
                                "<li>" +
                                tr ( "256 samples: Should only be used when 64 or 128 samples "
                                     "is causing issues." ) +
                                "</li>"
                                "</ul>" +
                                tr ( "Some sound card drivers do not allow the buffer delay to be changed "
                                     "from within %1. "
                                     "In this case the buffer delay setting is disabled and has to be "
                                     "changed using the sound card driver. On Windows, use the "
                                     "ASIO Device Settings button to open the driver settings panel. On Linux, "
                                     "use the JACK configuration tool to change the buffer size." )
                                    .arg ( APP_NAME ) +
                                "<br>" +
                                tr ( "If no buffer size is selected and all settings are disabled, this means a "
                                     "buffer size in use by the driver which does not match the values. %1 "
                                     "will still work with this setting but may have restricted "
                                     "performance." )
                                    .arg ( APP_NAME ) +
                                "<br>" +
                                tr ( "The actual buffer delay has influence on the connection, the "
                                     "current upload rate and the overall delay. The lower the buffer size, "
                                     "the higher the probability of a red light in the status indicator (drop "
                                     "outs) and the higher the upload rate and the lower the overall "
                                     "delay." ) +
                                "<br>" +
                                tr ( "The buffer setting is therefore a trade-off between audio "
                                     "quality and overall delay." );

    QString strSndCrdBufDelayTT = tr ( "If the buffer delay settings are "
                                       "disabled, it is prohibited by the audio driver to modify this "
                                       "setting from within %1. "
                                       "On Windows, press the ASIO Device Settings button to open the "
                                       "driver settings panel. On Linux, use the JACK configuration tool to "
                                       "change the buffer size." )
                                      .arg ( APP_NAME ) +
                                  TOOLTIP_COM_END_TEXT;

#if defined( _WIN32 ) && !defined( WITH_JACK )
    // Driver setup button
    QString strSndCardDriverSetup = "<b>" + tr ( "Sound card driver settings" ) + ":</b> " +
                                    tr ( "This opens the driver settings of your sound card. Some drivers "
                                         "allow you to change buffer settings, others like ASIO4ALL "
                                         "lets you choose input or outputs of your device(s). "
                                         "More information can be found on jamulus.io." );

    QString strSndCardDriverSetupTT = tr ( "Opens the driver settings. Note: %1 currently only supports devices "
                                           "with a sample rate of %2 Hz. "
                                           "You will not be able to select a driver/device which doesn't. "
                                           "For more help see jamulus.io." )
                                          .arg ( APP_NAME )
                                          .arg ( SYSTEM_SAMPLE_RATE_HZ ) +
                                      TOOLTIP_COM_END_TEXT;
#endif

    rbtBufferDelayPreferred->setWhatsThis ( strSndCrdBufDelay );
    rbtBufferDelayPreferred->setAccessibleName ( tr ( "64 samples setting radio button" ) );
    rbtBufferDelayPreferred->setToolTip ( strSndCrdBufDelayTT );
    rbtBufferDelayDefault->setWhatsThis ( strSndCrdBufDelay );
    rbtBufferDelayDefault->setAccessibleName ( tr ( "128 samples setting radio button" ) );
    rbtBufferDelayDefault->setToolTip ( strSndCrdBufDelayTT );
    rbtBufferDelaySafe->setWhatsThis ( strSndCrdBufDelay );
    rbtBufferDelaySafe->setAccessibleName ( tr ( "256 samples setting radio button" ) );
    rbtBufferDelaySafe->setToolTip ( strSndCrdBufDelayTT );

#if defined( _WIN32 ) && !defined( WITH_JACK )
    butDriverSetup->setWhatsThis ( strSndCardDriverSetup );
    butDriverSetup->setAccessibleName ( tr ( "ASIO Device Settings push button" ) );
    butDriverSetup->setToolTip ( strSndCardDriverSetupTT );
#endif

    // fancy skin
    lblSkin->setWhatsThis ( "<b>" + tr ( "Skin" ) + ":</b> " + tr ( "Select the skin to be used for the main window." ) );

    cbxSkin->setAccessibleName ( tr ( "Skin combo box" ) );

    // MeterStyle
    lblMeterStyle->setWhatsThis ( "<b>" + tr ( "Meter Style" ) + ":</b> " +
                                  tr ( "Select the meter style to be used for the level meters. The "
                                       "Bar (narrow) and LEDs (round, small) options only apply to the mixerboard. When "
                                       "Bar (narrow) is selected, the input meters are set to Bar (wide). When "
                                       "LEDs (round, small) is selected, the input meters are set to LEDs (round, big). "
                                       "The remaining options apply to the mixerboard and input meters." ) );

    cbxMeterStyle->setAccessibleName ( tr ( "Meter Style combo box" ) );

    // Interface Language
    lblLanguage->setWhatsThis ( "<b>" + tr ( "Language" ) + ":</b> " + tr ( "Select the language to be used for the user interface." ) );

    cbxLanguage->setAccessibleName ( tr ( "Language combo box" ) );

    // audio channels
    QString strAudioChannels = "<b>" + tr ( "Audio Channels" ) + ":</b> " +
                               tr ( "Selects the number of audio channels to be used for communication between "
                                    "client and server. There are three modes available:" ) +
                               "<ul>"
                               "<li>"
                               "<b>" +
                               tr ( "Mono" ) + "</b> " + tr ( "and" ) + " <b>" + tr ( "Stereo" ) + ":</b> " +
                               tr ( "These modes use "
                                    "one and two audio channels respectively." ) +
                               "</li>"
                               "<li>"
                               "<b>" +
                               tr ( "Mono in/Stereo-out" ) + ":</b> " +
                               tr ( "The audio signal sent to the server is mono but the "
                                    "return signal is stereo. This is useful if the "
                                    "sound card has the instrument on one input channel and the "
                                    "microphone on the other. In that case the two input signals "
                                    "can be mixed to one mono channel but the server mix is heard in "
                                    "stereo." ) +
                               "</li>"
                               "<li>" +
                               tr ( "Enabling " ) + "<b>" + tr ( "Stereo" ) + "</b> " +
                               tr ( " mode "
                                    "will increase your stream's data rate. Make sure your upload rate does not "
                                    "exceed the available upload speed of your internet connection." ) +
                               "</li>"
                               "</ul>" +
                               tr ( "In stereo streaming mode, no audio channel selection "
                                    "for the reverb effect will be available on the main window "
                                    "since the effect is applied to both channels in this case." );

    lblAudioChannels->setWhatsThis ( strAudioChannels );
    cbxAudioChannels->setWhatsThis ( strAudioChannels );
    cbxAudioChannels->setAccessibleName ( tr ( "Audio channels combo box" ) );

    // audio quality
    QString strAudioQuality = "<b>" + tr ( "Audio Quality" ) + ":</b> " +
                              tr ( "The higher the audio quality, the higher your audio stream's "
                                   "data rate. Make sure your upload rate does not exceed the "
                                   "available bandwidth of your internet connection." );

    lblAudioQuality->setWhatsThis ( strAudioQuality );
    cbxAudioQuality->setWhatsThis ( strAudioQuality );
    cbxAudioQuality->setAccessibleName ( tr ( "Audio quality combo box" ) );

    // new client fader level
    QString strNewClientLevel = "<b>" + tr ( "New Client Level" ) + ":</b> " +
                                tr ( "This setting defines the fader level of a newly "
                                     "connected client in percent. If a new client connects "
                                     "to the current server, they will get the specified initial "
                                     "fader level if no other fader level from a previous connection "
                                     "of that client was already stored." );

    lblNewClientLevel->setWhatsThis ( strNewClientLevel );
    edtNewClientLevel->setWhatsThis ( strNewClientLevel );
    edtNewClientLevel->setAccessibleName ( tr ( "New client level edit box" ) );

    // input boost
    QString strInputBoost = "<b>" + tr ( "Input Boost" ) + ":</b> " +
                            tr ( "This setting allows you to increase your input signal level "
                                 "by factors up to 10 (+20dB). "
                                 "If your sound is too quiet, first try to increase the level by "
                                 "getting closer to the microphone, adjusting your sound equipment "
                                 "or increasing levels in your operating system's input settings. "
                                 "Only if this fails, set a factor here. "
                                 "If your sound is too loud, sounds distorted and is clipping, this "
                                 "option will not help. Do not use it. The distortion will still be "
                                 "there. Instead, decrease your input level by getting farther away "
                                 "from your microphone, adjusting your sound equipment "
                                 "or by decreasing your operating system's input settings." );
    lblInputBoost->setWhatsThis ( strInputBoost );
//    cbxInputBoost->setWhatsThis ( strInputBoost );
//    cbxInputBoost->setAccessibleName ( tr ( "Input Boost combo box" ) );

    // custom directories
    QString strCustomDirectories = "<b>" + tr ( "Custom Directories" ) + ":</b> " +
                                   tr ( "If you need to add additional directories to the Connect dialog Directory drop down, "
                                        "you can enter the addresses here.<br>"
                                        "To remove a value, select it, delete the text in the input box, "
                                        "then move focus out of the control." );

    lblCustomDirectories->setWhatsThis ( strCustomDirectories );
    cbxCustomDirectories->setWhatsThis ( strCustomDirectories );
    cbxCustomDirectories->setAccessibleName ( tr ( "Custom Directories combo box" ) );

    // current connection status parameter
    QString audioUpstreamVal = "<b>" + tr ( "Audio Upstream Rate" ) + ":</b> " +
                           tr ( "Depends on the current audio packet size and "
                                "compression setting. Make sure that the upstream rate is not "
                                "higher than your available internet upload speed (check this with a "
                                "service such as speedtest.net)." );

    lblUpstreamValue->setWhatsThis ( audioUpstreamVal );
    grbUpstreamValue->setWhatsThis ( audioUpstreamVal );

    QString strNumMixerPanelRows =
        "<b>" + tr ( "Number of Mixer Panel Rows" ) + ":</b> " + tr ( "Adjust the number of rows used to arrange the mixer panel." );
    lblMixerRows->setWhatsThis ( strNumMixerPanelRows );
    spnMixerRows->setWhatsThis ( strNumMixerPanelRows );
    spnMixerRows->setAccessibleName ( tr ( "Number of Mixer Panel Rows spin box" ) );

    QString strDetectFeedback = "<b>" + tr ( "Feedback Protection" ) + ":</b> " +
                                tr ( "Enable feedback protection to detect acoustic feedback between "
                                     "microphone and speakers." );
    lblDetectFeedback->setWhatsThis ( strDetectFeedback );
    chbDetectFeedback->setWhatsThis ( strDetectFeedback );
    chbDetectFeedback->setAccessibleName ( tr ( "Feedback Protection check box" ) );

    // init driver button
#if defined( _WIN32 ) && !defined( WITH_JACK )
    butDriverSetup->setText ( tr ( "ASIO Device Settings" ) );
#else
    // no use for this button for MacOS/Linux right now or when using JACK -> hide it
    butDriverSetup->hide();
#endif

    // init audio in fader
//    sldAudioPan->setRange ( AUD_FADER_IN_MIN, AUD_FADER_IN_MAX );
    panDial->setRange ( AUD_FADER_IN_MIN, AUD_FADER_IN_MAX );
//    sldAudioPan->setTickInterval ( AUD_FADER_IN_MAX / 5 );
    UpdateAudioFaderSlider();

    // init delay and other information controls
    lblUpstreamValue->setText ( "---" );
    lblUpstreamUnit->setText ( "" );
    edtNewClientLevel->setValidator ( new QIntValidator ( 0, 100, this ) ); // % range from 0-100

    // init slider controls ---
    // network buffer sliders
    sldNetBuf->setRange ( MIN_NET_BUF_SIZE_NUM_BL, MAX_NET_BUF_SIZE_NUM_BL );
    sldNetBufServer->setRange ( MIN_NET_BUF_SIZE_NUM_BL, MAX_NET_BUF_SIZE_NUM_BL );
    UpdateJitterBufferFrame();

    // init sound card channel selection frame
    UpdateSoundDeviceChannelSelectionFrame();

    // Audio Channels combo box
    cbxAudioChannels->clear();
    cbxAudioChannels->addItem ( tr ( "Mono" ) );               // CC_MONO
    cbxAudioChannels->addItem ( tr ( "Mono-in/Stereo-out" ) ); // CC_MONO_IN_STEREO_OUT
    cbxAudioChannels->addItem ( tr ( "Stereo" ) );             // CC_STEREO
    cbxAudioChannels->setCurrentIndex ( static_cast<int> ( pClient->GetAudioChannels() ) );

    // Audio Quality combo box
    cbxAudioQuality->clear();
    cbxAudioQuality->addItem ( tr ( "Low" ) );    // AQ_LOW
    cbxAudioQuality->addItem ( tr ( "Normal" ) ); // AQ_NORMAL
    cbxAudioQuality->addItem ( tr ( "High" ) );   // AQ_HIGH
    cbxAudioQuality->setCurrentIndex ( static_cast<int> ( pClient->GetAudioQuality() ) );

    // GUI design (skin) combo box
    cbxSkin->clear();
    cbxSkin->addItem ( tr ( "Normal" ) );  // GD_STANDARD
    cbxSkin->addItem ( tr ( "Fancy" ) );   // GD_ORIGINAL
    cbxSkin->addItem ( tr ( "Compact" ) ); // GD_SLIMFADER
    cbxSkin->setCurrentIndex ( static_cast<int> ( pClient->GetGUIDesign() ) );

    // MeterStyle combo box
    cbxMeterStyle->clear();
    cbxMeterStyle->addItem ( tr ( "Bar (narrow)" ) );        // MT_BAR_NARROW
    cbxMeterStyle->addItem ( tr ( "Bar (wide)" ) );          // MT_BAR_WIDE
    cbxMeterStyle->addItem ( tr ( "LEDs (stripe)" ) );       // MT_LED_STRIPE
    cbxMeterStyle->addItem ( tr ( "LEDs (round, small)" ) ); // MT_LED_ROUND_SMALL
    cbxMeterStyle->addItem ( tr ( "LEDs (round, big)" ) );   // MT_LED_ROUND_BIG
    cbxMeterStyle->setCurrentIndex ( static_cast<int> ( pClient->GetMeterStyle() ) );

    // language combo box (corrects the setting if language not found)
    cbxLanguage->Init ( pSettings->strLanguage );

    // init custom directories combo box (max MAX_NUM_SERVER_ADDR_ITEMS entries)
    cbxCustomDirectories->setMaxCount ( MAX_NUM_SERVER_ADDR_ITEMS );
    cbxCustomDirectories->setInsertPolicy ( QComboBox::NoInsert );

    // update new client fader level edit box
    edtNewClientLevel->setText ( QString::number ( pSettings->iNewClientFaderLevel ) );

//    // Input Boost combo box
//    cbxInputBoost->clear();
//    cbxInputBoost->addItem ( tr ( "None" ) );
//    for ( int i = 2; i <= 10; i++ )
//    {
//        cbxInputBoost->addItem ( QString ( "%1x" ).arg ( i ) );
//    }
    // factor is 1-based while index is 0-based:
//    cbxInputBoost->setCurrentIndex ( pSettings->iInputBoost - 1 );
    dialInputBoost->setValue( pSettings->iInputBoost );

    // init number of mixer rows
    spnMixerRows->setValue ( pSettings->iNumMixerPanelRows );

    // update feedback detection
    chbDetectFeedback->setCheckState ( pSettings->bEnableFeedbackDetection ? Qt::Checked : Qt::Unchecked );

    // update enable small network buffers check box
    chbEnableOPUS64->setCheckState ( pClient->GetEnableOPUS64() ? Qt::Checked : Qt::Unchecked );

    // set text for sound card buffer delay radio buttons
    rbtBufferDelayPreferred->setText ( GenSndCrdBufferDelayString ( FRAME_SIZE_FACTOR_PREFERRED * SYSTEM_FRAME_SIZE_SAMPLES ) );

    rbtBufferDelayDefault->setText ( GenSndCrdBufferDelayString ( FRAME_SIZE_FACTOR_DEFAULT * SYSTEM_FRAME_SIZE_SAMPLES ) );

    rbtBufferDelaySafe->setText ( GenSndCrdBufferDelayString ( FRAME_SIZE_FACTOR_SAFE * SYSTEM_FRAME_SIZE_SAMPLES ) );

    // sound card buffer delay inits
    SndCrdBufferDelayButtonGroup.addButton ( rbtBufferDelayPreferred );
    SndCrdBufferDelayButtonGroup.addButton ( rbtBufferDelayDefault );
    SndCrdBufferDelayButtonGroup.addButton ( rbtBufferDelaySafe );

    UpdateSoundCardFrame();

    // Add help text to controls -----------------------------------------------
    // Musician Profile
    QString strFaderTag = "<b>" + tr ( "Musician Profile" ) + ":</b> " +
                          tr ( "Write your name or an alias here so the other musicians you want to "
                               "play with know who you are. You may also add a picture of the instrument "
                               "you play and a flag of the country or region you are located in. "
                               "Your city and skill level playing your instrument may also be added." ) +
                          "<br>" +
                          tr ( "What you set here will appear at your fader on the mixer "
                               "board when you are connected to a %1 server. This tag will "
                               "also be shown at each client which is connected to the same server as "
                               "you." )
                              .arg ( APP_NAME );

    plblAlias->setWhatsThis ( strFaderTag );
    pedtAlias->setAccessibleName ( tr ( "Alias or name edit box" ) );
    plblInstrument->setWhatsThis ( strFaderTag );
    pcbxInstrument->setAccessibleName ( tr ( "Instrument picture button" ) );
    plblCountry->setWhatsThis ( strFaderTag );
    pcbxCountry->setAccessibleName ( tr ( "Country/region flag button" ) );
    plblCity->setWhatsThis ( strFaderTag );
    pedtCity->setAccessibleName ( tr ( "City edit box" ) );
    plblSkill->setWhatsThis ( strFaderTag );
    pcbxSkill->setAccessibleName ( tr ( "Skill level combo box" ) );

    // Instrument pictures combo box -------------------------------------------
    // add an entry for all known instruments
    for ( int iCurInst = 0; iCurInst < CInstPictures::GetNumAvailableInst(); iCurInst++ )
    {
        // create a combo box item with text, image and background color
        QColor InstrColor;

        pcbxInstrument->addItem ( QIcon ( CInstPictures::GetResourceReference ( iCurInst ) ), CInstPictures::GetName ( iCurInst ), iCurInst );

        switch ( CInstPictures::GetCategory ( iCurInst ) )
        {
        case CInstPictures::IC_OTHER_INSTRUMENT:
            InstrColor = QColor ( Qt::blue );
            break;
        case CInstPictures::IC_WIND_INSTRUMENT:
            InstrColor = QColor ( Qt::green );
            break;
        case CInstPictures::IC_STRING_INSTRUMENT:
            InstrColor = QColor ( Qt::red );
            break;
        case CInstPictures::IC_PLUCKING_INSTRUMENT:
            InstrColor = QColor ( Qt::cyan );
            break;
        case CInstPictures::IC_PERCUSSION_INSTRUMENT:
            InstrColor = QColor ( Qt::white );
            break;
        case CInstPictures::IC_KEYBOARD_INSTRUMENT:
            InstrColor = QColor ( Qt::yellow );
            break;
        case CInstPictures::IC_MULTIPLE_INSTRUMENT:
            InstrColor = QColor ( Qt::black );
            break;
        }

        InstrColor.setAlpha ( 10 );
        pcbxInstrument->setItemData ( iCurInst, InstrColor, Qt::BackgroundRole );
    }

    // sort the items in alphabetical order
    pcbxInstrument->model()->sort ( 0 );

    // Country flag icons combo box --------------------------------------------
    // add an entry for all known country flags
    for ( int iCurCntry = static_cast<int> ( QLocale::AnyCountry ); iCurCntry < static_cast<int> ( QLocale::LastCountry ); iCurCntry++ )
    {
        // exclude the "None" entry since it is added after the sorting
        if ( static_cast<QLocale::Country> ( iCurCntry ) == QLocale::AnyCountry )
        {
            continue;
        }

        if ( !CLocale::IsCountryCodeSupported ( iCurCntry ) )
        {
            // The current Qt version which is the base for the loop may support
            // more country codes than our protocol does. Therefore, skip
            // the unsupported options to avoid surprises.
            continue;
        }

        // get current country enum
        QLocale::Country eCountry = static_cast<QLocale::Country> ( iCurCntry );

        // try to load icon from resource file name
        QIcon CurFlagIcon;
        CurFlagIcon.addFile ( CLocale::GetCountryFlagIconsResourceReference ( eCountry ) );

        // only add the entry if a flag is available
        if ( !CurFlagIcon.isNull() )
        {
            // create a combo box item with text and image
            pcbxCountry->addItem ( QIcon ( CurFlagIcon ), QLocale::countryToString ( eCountry ), iCurCntry );
        }
    }

    // sort country combo box items in alphabetical order
    pcbxCountry->model()->sort ( 0, Qt::AscendingOrder );

    // the "None" country gets a special icon and is the very first item
    QIcon FlagNoneIcon;
    FlagNoneIcon.addFile ( ":/png/flags/res/flags/flagnone.png" );
    pcbxCountry->insertItem ( 0, FlagNoneIcon, tr ( "None" ), static_cast<int> ( QLocale::AnyCountry ) );

    // Skill level combo box ---------------------------------------------------
    // create a pixmap showing the skill level colors
    QPixmap SLPixmap ( 16, 11 ); // same size as the country flags

    SLPixmap.fill ( QColor::fromRgb ( RGBCOL_R_SL_NOT_SET, RGBCOL_G_SL_NOT_SET, RGBCOL_B_SL_NOT_SET ) );

    pcbxSkill->addItem ( QIcon ( SLPixmap ), tr ( "None" ), SL_NOT_SET );

    SLPixmap.fill ( QColor::fromRgb ( RGBCOL_R_SL_BEGINNER, RGBCOL_G_SL_BEGINNER, RGBCOL_B_SL_BEGINNER ) );

    pcbxSkill->addItem ( QIcon ( SLPixmap ), tr ( "Beginner" ), SL_BEGINNER );

    SLPixmap.fill ( QColor::fromRgb ( RGBCOL_R_SL_INTERMEDIATE, RGBCOL_G_SL_INTERMEDIATE, RGBCOL_B_SL_INTERMEDIATE ) );

    pcbxSkill->addItem ( QIcon ( SLPixmap ), tr ( "Intermediate" ), SL_INTERMEDIATE );

    SLPixmap.fill ( QColor::fromRgb ( RGBCOL_R_SL_SL_PROFESSIONAL, RGBCOL_G_SL_SL_PROFESSIONAL, RGBCOL_B_SL_SL_PROFESSIONAL ) );

    pcbxSkill->addItem ( QIcon ( SLPixmap ), tr ( "Expert" ), SL_PROFESSIONAL );

// end of settings stuff ---------------------------------------------------------------



    // Connections -------------------------------------------------------------
    // push buttons
    QObject::connect ( butConnect, &QPushButton::clicked, this, &CClientDlg::OnConnectDisconBut );
    QObject::connect ( butNewStart, &QPushButton::clicked, this, &CClientDlg::OnNewStartClicked );
    QObject::connect ( downloadLinkButton, &QPushButton::clicked, this, &CClientDlg::OnDownloadUpdateClicked );
    QObject::connect ( inviteComboBox, &QComboBox::activated, this, &CClientDlg::OnInviteBoxActivated );
    QObject::connect ( checkUpdateButton, &QPushButton::clicked, this, &CClientDlg::OnCheckForUpdate );


    // session chat stuff
    QObject::connect ( edtLocalInputText, &QLineEdit::textChanged, this, &CClientDlg::OnLocalInputTextTextChanged );
    QObject::connect ( this, &CClientDlg::NewLocalInputText, this, &CClientDlg::OnNewLocalInputText );
    QObject::connect ( butSend, &QPushButton::clicked, this, &CClientDlg::OnSendText );
    QObject::connect ( edtLocalInputText, &QLineEdit::returnPressed, this, &CClientDlg::OnSendText );
    QObject::connect ( txvChatWindow, &QTextBrowser::anchorClicked, this, &CClientDlg::OnAnchorClicked );

    // check boxes
    QObject::connect ( chbSettings, &QCheckBox::stateChanged, this, &CClientDlg::OnSettingsStateChanged );

//    QObject::connect ( chbPubJam, &QCheckBox::stateChanged, this, &CClientDlg::OnPubConnectStateChanged );

    QObject::connect ( chbChat, &QCheckBox::stateChanged, this, &CClientDlg::OnChatStateChanged );

    QObject::connect ( chbLocalMute, &QCheckBox::stateChanged, this, &CClientDlg::OnLocalMuteStateChanged );

    // timers
    QObject::connect ( &TimerSigMet, &QTimer::timeout, this, &CClientDlg::OnTimerSigMet );

    QObject::connect ( &TimerBuffersLED, &QTimer::timeout, this, &CClientDlg::OnTimerBuffersLED );

    QObject::connect ( &TimerStatus, &QTimer::timeout, this, &CClientDlg::OnTimerStatus );

    QObject::connect ( &TimerPing, &QTimer::timeout, this, &CClientDlg::OnTimerPing );

    QObject::connect ( &TimerCheckAudioDeviceOk, &QTimer::timeout, this, &CClientDlg::OnTimerCheckAudioDeviceOk );

    QObject::connect ( &TimerDetectFeedback, &QTimer::timeout, this, &CClientDlg::OnTimerDetectFeedback );

    QObject::connect ( sldAudioReverb, &QDial::valueChanged, this, &CClientDlg::OnAudioReverbValueChanged );

    // radio buttons
    QObject::connect ( rbtReverbSelL, &QRadioButton::clicked, this, &CClientDlg::OnReverbSelLClicked );

    QObject::connect ( rbtReverbSelR, &QRadioButton::clicked, this, &CClientDlg::OnReverbSelRClicked );

    // other
    QObject::connect ( pClient, &CClient::ConClientListMesReceived, this, &CClientDlg::OnConClientListMesReceived );

    QObject::connect ( pClient, &CClient::Disconnected, this, &CClientDlg::OnDisconnected );

    QObject::connect ( pClient, &CClient::ChatTextReceived, this, &CClientDlg::OnChatTextReceived );

    QObject::connect ( pClient, &CClient::ClientIDReceived, this, &CClientDlg::OnClientIDReceived );

    QObject::connect ( pClient, &CClient::MuteStateHasChangedReceived, this, &CClientDlg::OnMuteStateHasChangedReceived );

    QObject::connect ( pClient, &CClient::RecorderStateReceived, this, &CClientDlg::OnRecorderStateReceived );

    // This connection is a special case. On receiving a licence required message via the
    // protocol, a modal licence dialog is opened. Since this blocks the thread, we need
    // a queued connection to make sure the core protocol mechanism is not blocked, too.
    qRegisterMetaType<ELicenceType> ( "ELicenceType" );
    QObject::connect ( pClient, &CClient::LicenceRequired, this, &CClientDlg::OnLicenceRequired, Qt::QueuedConnection );

    QObject::connect ( pClient, &CClient::PingTimeReceived, this, &CClientDlg::OnPingTimeResult );

//    QObject::connect ( pClient, &CClient::CLServerListReceived, this, &CClientDlg::OnCLServerListReceived );

//    QObject::connect ( pClient, &CClient::CLRedServerListReceived, this, &CClientDlg::OnCLRedServerListReceived );

//    QObject::connect ( pClient, &CClient::CLConnClientsListMesReceived, this, &CClientDlg::OnCLConnClientsListMesReceived );

//    QObject::connect ( pClient, &CClient::CLPingTimeWithNumClientsReceived, this, &CClientDlg::OnCLPingTimeWithNumClientsReceived );

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
    QObject::connect ( this, &CClientDlg::GUIDesignChanged, this, &CClientDlg::OnGUIDesignChanged );

    QObject::connect ( this, &CClientDlg::MeterStyleChanged, this, &CClientDlg::OnMeterStyleChanged );

    QObject::connect ( this, &CClientDlg::AudioChannelsChanged, this, &CClientDlg::OnAudioChannelsChanged );

//    QObject::connect ( this, &CClientDlg::CustomDirectoriesChanged, &ConnectDlg, &CConnectDlg::OnCustomDirectoriesChanged );

    QObject::connect ( this, &CClientDlg::NumMixerPanelRowsChanged, this, &CClientDlg::OnNumMixerPanelRowsChanged );

//    QObject::connect ( this, &CClientDlg::SendTabChange, this, &CClientDlg::OnMakeTabChange );
    // end of settings slots --------------------------------


    QObject::connect ( MainMixerBoard, &CAudioMixerBoard::ChangeChanGain, this, &CClientDlg::OnChangeChanGain );

    QObject::connect ( MainMixerBoard, &CAudioMixerBoard::ChangeChanPan, this, &CClientDlg::OnChangeChanPan );

    QObject::connect ( MainMixerBoard, &CAudioMixerBoard::NumClientsChanged, this, &CClientDlg::OnNumClientsChanged );


//    QObject::connect ( &ConnectDlg, &CConnectDlg::ReqServerListQuery, this, &CClientDlg::OnReqServerListQuery );

    // note that this connection must be a queued connection, otherwise the server list ping
    // times are not accurate and the client list may not be retrieved for all servers listed
    // (it seems the sendto() function needs to be called from different threads to fire the
    // packet immediately and do not collect packets before transmitting)
//    QObject::connect ( &ConnectDlg, &CConnectDlg::CreateCLServerListPingMes, this, &CClientDlg::OnCreateCLServerListPingMes, Qt::QueuedConnection );

//    QObject::connect ( &ConnectDlg, &CConnectDlg::CreateCLServerListReqVerAndOSMes, this, &CClientDlg::OnCreateCLServerListReqVerAndOSMes );

//    QObject::connect ( &ConnectDlg,
//                       &CConnectDlg::CreateCLServerListReqConnClientsListMes,
//                       this,
//                       &CClientDlg::OnCreateCLServerListReqConnClientsListMes );

//    QObject::connect ( &ConnectDlg, &CConnectDlg::accepted, this, &CClientDlg::OnConnectDlgAccepted );
//    QObject::connect ( &BasicConnectDlg, &CBasicConnectDlg::accepted, this, &CClientDlg::OnBasicConnectDlgAccepted );
    // with sessionInputField ....????
//    QObject::connect ( this, &CBasicConnectDlg::accepted, this, &CClientDlg::OnBasicConnectDlgAccepted );

    QObject::connect ( sessionCancelButton, &QPushButton::clicked, this, &CClientDlg::OnJoinCancelClicked );
    QObject::connect ( sessionConnectButton, &QPushButton::clicked, this, &CClientDlg::OnJoinConnectClicked );

    // ==================================================================================================
    // SETTINGS SLOTS ========
    // ==================================================================================================
    // Connections -------------------------------------------------------------
    // timers
    QObject::connect ( &TimerStatus, &QTimer::timeout, this, &CClientDlg::OnTimerStatus );

    // slider controls
    QObject::connect ( sldNetBuf, &QSlider::valueChanged, this, &CClientDlg::OnNetBufValueChanged );

    QObject::connect ( sldNetBufServer, &QSlider::valueChanged, this, &CClientDlg::OnNetBufServerValueChanged );

    // check boxes
    QObject::connect ( chbAutoJitBuf, &QCheckBox::stateChanged, this, &CClientDlg::OnAutoJitBufStateChanged );

    QObject::connect ( chbEnableOPUS64, &QCheckBox::stateChanged, this, &CClientDlg::OnEnableOPUS64StateChanged );

    QObject::connect ( chbDetectFeedback, &QCheckBox::stateChanged, this, &CClientDlg::OnFeedbackDetectionChanged );

//    // line edits
//    QObject::connect ( edtNewClientLevel, &QLineEdit::editingFinished, this, &CClientDlg::OnNewClientLevelEditingFinished );
    // line edits
//    QObject::connect ( newInputLevelDial, &QLineEdit::editingFinished, this, &CClientDlg::OnNewClientLevelEditingFinished );
    QObject::connect ( newInputLevelDial,
                       &QSlider::valueChanged,
                       this,
                       &CClientDlg::OnNewClientLevelChanged );

    // combo boxes
    QObject::connect ( cbxSoundcard,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnSoundcardActivated );

    QObject::connect ( cbxLInChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnLInChanActivated );

    QObject::connect ( cbxRInChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnRInChanActivated );

    QObject::connect ( cbxLOutChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnLOutChanActivated );

    QObject::connect ( cbxROutChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnROutChanActivated );

    QObject::connect ( cbxAudioChannels,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnAudioChannelsActivated );

    QObject::connect ( cbxAudioQuality,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnAudioQualityActivated );

    QObject::connect ( cbxSkin,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnGUIDesignActivated );

    QObject::connect ( cbxMeterStyle,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnMeterStyleActivated );

    QObject::connect ( cbxCustomDirectories->lineEdit(), &QLineEdit::editingFinished, this, &CClientDlg::OnCustomDirectoriesEditingFinished );

    QObject::connect ( cbxCustomDirectories,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnCustomDirectoriesEditingFinished );

    QObject::connect ( cbxLanguage, &CLanguageComboBox::LanguageChanged, this, &CClientDlg::OnLanguageChanged );

//    QObject::connect ( cbxInputBoost,
//                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
//                       this,
//                       &CClientDlg::OnInputBoostChanged );

    QObject::connect ( dialInputBoost,
                       &QSlider::valueChanged,
                       this,
                       &CClientDlg::OnInputBoostChanged );

    // buttons
#if defined( _WIN32 ) && !defined( WITH_JACK )
    // Driver Setup button is only available for Windows when JACK is not used
    QObject::connect ( butDriverSetup, &QPushButton::clicked, this, &CClientDlg::OnDriverSetupClicked );
#endif

    // misc
    // sliders
//    QObject::connect ( sldAudioPan, &QSlider::valueChanged, this, &CClientDlg::OnAudioPanValueChanged );
    // panDial
    QObject::connect ( panDial, &QSlider::valueChanged, this, &CClientDlg::OnAudioPanValueChanged );

    QObject::connect ( &SndCrdBufferDelayButtonGroup,
                       static_cast<void ( QButtonGroup::* ) ( QAbstractButton* )> ( &QButtonGroup::buttonClicked ),
                       this,
                       &CClientDlg::OnSndCrdBufferDelayButtonGroupClicked );

    // spinners
    QObject::connect ( spnMixerRows,
                       static_cast<void ( QSpinBox::* ) ( int )> ( &QSpinBox::valueChanged ),
                       this,
                       &CClientDlg::NumMixerPanelRowsChanged );

    // Musician Profile
    QObject::connect ( pedtAlias, &QLineEdit::textChanged, this, &CClientDlg::OnAliasTextChanged );

    QObject::connect ( pcbxInstrument,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnInstrumentActivated );

    QObject::connect ( pcbxCountry,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientDlg::OnCountryActivated );

    QObject::connect ( pedtCity, &QLineEdit::textChanged, this, &CClientDlg::OnCityTextChanged );

    QObject::connect ( pcbxSkill, static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ), this, &CClientDlg::OnSkillActivated );

//    QObject::connect ( qNam, SIGNAL(finished(QNetworkReply *)), this, &CClientDlg::replyFinished );
//    QObject::connect( qNam, &QNetworkReply::finished, this, &CClientDlg::replyFinished );

    // Timers ------------------------------------------------------------------
    // start timer for status bar
    TimerStatus.start ( DISPLAY_UPDATE_TIME );

    // END OF SETTINGS SLOTS =======================================================================================


    // Initializations which have to be done after the signals are connected ---
    // start timer for status bar
    TimerStatus.start ( LED_BAR_UPDATE_TIME_MS );

//    // restore connect dialog
//    if ( pSettings->bWindowWasShownConnect )
//    {
//         ShowConnectionSetupDialog();
//    }

//    // restore basic connect dialog
//    if ( pSettings->bWindowWasShownBasicConnect )
//    {
//        ShowBasicConnectionSetupDialog();
//    }
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
//    pSettings->vecWindowPosSettings = ClientSettingsDlg.saveGeometry();
//    pSettings->vecWindowPosChat     = ChatDlg.saveGeometry();
//    pSettings->vecWindowPosConnect  = ConnectDlg.saveGeometry();
    pSettings->vecWindowPosBasicConnect  = BasicConnectDlg.saveGeometry();

//    pSettings->bWindowWasShownSettings = ClientSettingsDlg.isVisible();
//    pSettings->bWindowWasShownChat     = ChatDlg.isVisible();
//    pSettings->bWindowWasShownConnect  = ConnectDlg.isVisible();
    pSettings->bWindowWasShownConnect  = BasicConnectDlg.isVisible();

    // if settings/connect dialog or chat dialog is open, close it
//    ClientSettingsDlg.close();
//    ChatDlg.close();
//    ConnectDlg.close();
    BasicConnectDlg.close();
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

//void CClientDlg::OnConnectDlgAccepted()
//{
//    // We had an issue that the accepted signal was emit twice if a list item was double
//    // clicked in the connect dialog. To avoid this we introduced a flag which makes sure
//    // we process the accepted signal only once after the dialog was initially shown.
//    if ( bConnectDlgWasShown )
//    {
//        // get the address from the connect dialog
//        QString strSelectedAddress = ConnectDlg.GetSelectedAddress();

//        // only store new host address in our data base if the address is
//        // not empty and it was not a server list item (only the addresses
//        // typed in manually are stored by definition)
//        if ( !strSelectedAddress.isEmpty() && !ConnectDlg.GetServerListItemWasChosen() )
//        {
//            // store new address at the top of the list, if the list was already
//            // full, the last element is thrown out
//            pSettings->vstrIPAddress.StringFiFoWithCompare ( strSelectedAddress );
//        }

//        // get name to be set in audio mixer group box title
//        QString strMixerBoardLabel;

//        if ( ConnectDlg.GetServerListItemWasChosen() )
//        {
//            // in case a server in the server list was chosen,
//            // display the server name of the server list
//            strMixerBoardLabel = ConnectDlg.GetSelectedServerName();
//        }
//        else
//        {
//            // an item of the server address combo box was chosen,
//            // just show the address string as it was entered by the
//            // user
//            strMixerBoardLabel = strSelectedAddress;

//            // special case: if the address is empty, we substitute the default
//            // directory address so that a user who just pressed the connect
//            // button without selecting an item in the table or manually entered an
//            // address gets a successful connection
//            if ( strSelectedAddress.isEmpty() )
//            {
//                strSelectedAddress = DEFAULT_SERVER_ADDRESS;
//                strMixerBoardLabel = tr ( "%1 Directory" ).arg ( DirectoryTypeToString ( AT_DEFAULT ) );
//            }
//        }

//        // first check if we are already connected, if this is the case we have to
//        // disconnect the old server first
//        if ( pClient->IsRunning() )
//        {
//            Disconnect();
//        }

//        // initiate connection
//        Connect ( strSelectedAddress, strMixerBoardLabel );

//        // reset flag
//        bConnectDlgWasShown = false;
//    }
//}

void CClientDlg::OnJoinCancelClicked()
{
    HideJoinWidget();
}

void CClientDlg::OnJoinConnectClicked()
{
    strSelectedAddress = NetworkUtil::FixAddress ( joinFieldEdit->text() );

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

void CClientDlg::OnBasicConnectDlgAccepted()
{
    // We had an issue that the accepted signal was emit twice if a list item was double
    // clicked in the connect dialog. To avoid this we introduced a flag which makes sure
    // we process the accepted signal only once after the dialog was initially shown.
    if ( bBasicConnectDlgWasShown )
    {
        // get the address from the connect dialog
        QString strSelectedAddress = BasicConnectDlg.GetSelectedAddress();

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

        // reset flag
        bBasicConnectDlgWasShown = false;
    }
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
    QString body = tr("You have an invite to play on Koord.Live.\n\n") +
                    tr("Copy (don't Click!) the Session Link and paste in the Koord app.\n") +
                    tr("Session Link: %1 \n\n").arg(strSelectedAddress) +
                    tr("If you don't have the free Koord app installed yet,\n") +
                    tr("go to https://koord.live/downloads and follow the links.");

    if ( text.contains( "Copy Session Link" ) )
    {
        inviteComboBox->setCurrentIndex(0);
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(strSelectedAddress);
        QToolTip::showText( inviteComboBox->mapToGlobal( QPoint( 0, 0 ) ), "Link Copied!" );
    }
    else if ( text.contains( "Share via Email" ) )
    {
        inviteComboBox->setCurrentIndex(0);
        QDesktopServices::openUrl(QUrl("mailto:?subject=" + subject + "&body=" + body, QUrl::TolerantMode));
    }
    else if ( text.contains( "Share via Whatsapp" ) )
    {
//        qInfo() << "Share via Whatsapp sleecte";
        inviteComboBox->setCurrentIndex(0);
        QDesktopServices::openUrl(QUrl("https://api.whatsapp.com/send?text=" + body, QUrl::TolerantMode));
    }
}

void CClientDlg::OnNewStartClicked()
{
    // just open website for now
    QDesktopServices::openUrl(QUrl("https://koord.live", QUrl::TolerantMode));
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

void CClientDlg::OnChatTextReceived ( QString strChatText )
{
    this->AddChatText ( strChatText );

    // Open chat dialog. If a server welcome message is received, we force
    // the dialog to be upfront in case a licence text is shown. For all
    // other new chat texts we do not want to force the dialog to be upfront
    // always when a new message arrives since this is annoying.
//    ShowChatWindow ( ( strChatText.indexOf ( WELCOME_MESSAGE_PREFIX ) == 0 ) );

    UpdateDisplay();
}

void CClientDlg::OnLicenceRequired ( ELicenceType eLicenceType )
{
    // right now only the creative common licence is supported
    if ( eLicenceType == LT_CREATIVECOMMONS )
    {
        CLicenceDlg LicenceDlg;

        // mute the client output stream
        pClient->SetMuteOutStream ( true );

        // Open the licence dialog and check if the licence was accepted. In
        // case the dialog is just closed or the decline button was pressed,
        // disconnect from that server.
        if ( !LicenceDlg.exec() )
        {
            Disconnect();
        }

        // unmute the client output stream if local mute button is not pressed
        if ( chbLocalMute->checkState() == Qt::Unchecked )
        {
            pClient->SetMuteOutStream ( false );
        }
    }
}

void CClientDlg::OnConClientListMesReceived ( CVector<CChannelInfo> vecChanInfo )
{
    // show channel numbers if --ctrlmidich is used (#241, #95)
    if ( bMIDICtrlUsed )
    {
        for ( int i = 0; i < vecChanInfo.Size(); i++ )
        {
            vecChanInfo[i].strName.prepend ( QString().setNum ( vecChanInfo[i].iChanID ) + ":" );
        }
    }

    // update mixer board with the additional client infos
    MainMixerBoard->ApplyNewConClientList ( vecChanInfo );
    // set session status bar with address
    sessionStatusLabel->setText("CONNECTED");
    sessionStatusLabel->setStyleSheet ( "QLabel { color: green; font: bold; }" );

}

void CClientDlg::OnNumClientsChanged ( int iNewNumClients )
{
    // update window title
    SetMyWindowTitle ( iNewNumClients );
}

//void CClientDlg::OnOpenAudioNetSettings() { ShowGeneralSettings ( SETTING_TAB_AUDIONET ); }

//void CClientDlg::OnOpenAdvancedSettings() { ShowGeneralSettings ( SETTING_TAB_ADVANCED ); }

//void CClientDlg::OnOpenUserProfileSettings() { ShowGeneralSettings ( SETTING_TAB_USER ); }

// session chat slot stuff
void CClientDlg::OnLocalInputTextTextChanged ( const QString& strNewText )
{
    // check and correct length
    if ( strNewText.length() > MAX_LEN_CHAT_TEXT )
    {
        // text is too long, update control with shortened text
        edtLocalInputText->setText ( strNewText.left ( MAX_LEN_CHAT_TEXT ) );
    }
}

void CClientDlg::OnSendText()
{
    // send new text and clear line afterwards, do not send an empty message
    if ( !edtLocalInputText->text().isEmpty() )
    {
        emit NewLocalInputText ( edtLocalInputText->text() );
        edtLocalInputText->clear();
    }
}

void CClientDlg::OnClearChatHistory()
{
    // clear chat window
    txvChatWindow->clear();
}

void CClientDlg::AddChatText ( QString strChatText )
{
    // notify accessibility plugin that text has changed
    QAccessible::updateAccessibility ( new QAccessibleValueChangeEvent ( txvChatWindow, strChatText ) );

    // analyze strChatText to check if hyperlink (limit ourselves to http(s)://) but do not
    // replace the hyperlinks if any HTML code for a hyperlink was found (the user has done the HTML
    // coding hisself and we should not mess with that)
    if ( !strChatText.contains ( QRegularExpression ( "href\\s*=|src\\s*=" ) ) )
    {
        // searches for all occurrences of http(s) and cuts until a space (\S matches any non-white-space
        // character and the + means that matches the previous element one or more times.)
        // This regex now contains three parts:
        // - https?://\\S+ matches as much non-whitespace as possible after the http:// or https://,
        //   subject to the next two parts, which exclude terminating punctuation
        // - (?<![!\"'()+,.:;<=>?\\[\\]{}]) is a negative look-behind assertion that disallows the match
        //   from ending with one of the characters !"'()+,.:;<=>?[]{}
        // - (?<!\\?[!\"'()+,.:;<=>?\\[\\]{}]) is a negative look-behind assertion that disallows the match
        //   from ending with a ? followed by one of the characters !"'()+,.:;<=>?[]{}
        // These last two parts must be separate, as a look-behind assertion must be fixed length.
#define PUNCT_NOEND_URL "[!\"'()+,.:;<=>?\\[\\]{}]"
        strChatText.replace ( QRegularExpression ( "(https?://\\S+(?<!" PUNCT_NOEND_URL ")(?<!\\?" PUNCT_NOEND_URL "))" ),
                              "<a href=\"\\1\">\\1</a>" );
    }

    // add new text in chat window
    txvChatWindow->append ( strChatText );
}

void CClientDlg::OnAnchorClicked ( const QUrl& Url )
{
    // only allow http(s) URLs to be opened in an external browser
    if ( Url.scheme() == QLatin1String ( "https" ) || Url.scheme() == QLatin1String ( "http" ) )
    {
        if ( QMessageBox::question ( this,
                                     APP_NAME,
                                     tr ( "Do you want to open the link '%1' in your browser?" ).arg ( "<b>" + Url.toString() + "</b>" ),
                                     QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
        {
            QDesktopServices::openUrl ( Url );
        }
    }
}
// end session chat stuff -------------------------------------------------------------

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

#if defined( Q_OS_MACX )
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

//void CClientDlg::ShowConnectionSetupDialog()
//{
//    // show connect dialog
//    bConnectDlgWasShown = true;
//    ConnectDlg.show();
//    ConnectDlg.setWindowTitle ( MakeClientNameTitle ( tr ( "Connect" ), pClient->strClientName ) );

//    // make sure dialog is upfront and has focus
//    ConnectDlg.raise();
//    ConnectDlg.activateWindow();
//}

//void CClientDlg::ShowBasicConnectionSetupDialog()
//{
//    // show connect dialog
//    bBasicConnectDlgWasShown = true;
//    BasicConnectDlg.show();
//    BasicConnectDlg.setWindowTitle ( MakeClientNameTitle ( tr ( "Connect" ), pClient->strClientName ) );

//    // make sure dialog is upfront and has focus
//    BasicConnectDlg.raise();
//    BasicConnectDlg.activateWindow();
//}

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

//void CClientDlg::ShowGeneralSettings ( int iTab )
//{
//    // open general settings dialog
//    emit SendTabChange ( iTab );
//    ClientSettingsDlg.show();
//    ClientSettingsDlg.setWindowTitle ( MakeClientNameTitle ( tr ( "Settings" ), pClient->strClientName ) );

//    // make sure dialog is upfront and has focus
//    ClientSettingsDlg.raise();
//    ClientSettingsDlg.activateWindow();
//}

//void CClientDlg::ShowChatWindow ( const bool bForceRaise )
//{
//    ChatDlg.show();
//    ChatDlg.setWindowTitle ( MakeClientNameTitle ( tr ( "Chat" ), pClient->strClientName ) );

//    if ( bForceRaise )
//    {
//        // make sure dialog is upfront and has focus
//        ChatDlg.showNormal();
//        ChatDlg.raise();
//        ChatDlg.activateWindow();
//    }

//    UpdateDisplay();
//}

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

//void CClientDlg::OnPubConnectStateChanged ( int value )
//{
//    if ( !bConnectDlgWasShown )
//    {
//        ShowConnectionSetupDialog();
//    }
//    else
//    {
//        bConnectDlgWasShown = false;
//        ConnectDlg.close();
//    }
//}

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

        QObject::connect ( chb, &QCheckBox::stateChanged, this, &CClientDlg::OnFeedbackDetectionChanged );

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
        UpdateUploadRate(); // set ping time result to settings tab

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
    UpdateSoundDeviceChannelSelectionFrame();
}

//void CClientDlg::OnCLPingTimeWithNumClientsReceived ( CHostAddress InetAddr, int iPingTime, int iNumClients )
//{
//    // update connection dialog server list
//    ConnectDlg.SetPingTimeAndNumClientsResult ( InetAddr, iPingTime, iNumClients );
//}

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

        // enable chat widgets
        butSend->setEnabled(true);
        edtLocalInputText->setEnabled(true);

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
        //FIXME - unhardcode test url
        QUrl url("https://test.koord.live/sess/sessionvideourl/");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("test-koord-login", "test17d3moin3s");

        QRegularExpression rx_sessaddr("^(([a-z]*[0-9]*\\.*)+):([0-9]+)$");
//        qInfo() << ">>> strSelectedaddress = " << strSelectedAddress;
        QRegularExpressionMatch reg_match = rx_sessaddr.match(strSelectedAddress);

        QString hostname;
        QString port;
        if (reg_match.hasMatch()) {
            hostname = reg_match.captured(1);
            port = reg_match.captured(3);
        }
//        qInfo() << ">>> hostname = " << hostname;
//        qInfo() << ">>> port = " << port;

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
//                qInfo() << ">>> CONNECT: " << reply->error();

                // if reply - no error
                // parse the JSON response
                QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
                QJsonObject jsonObject = jsonResponse.object();
                QString tmp_str = jsonObject.value("video_url").toString();

                // set the video url and update QML side
#if defined(Q_OS_MACX) || defined(Q_OS_IOS)
                quickView->setSource(QUrl("qrc:/webview.qml"));
#elif defined(ANDROID)
//                quickWidget->setSource(QUrl("qrc:/androidwebview.qml"));
#else
                quickView->setSource(QUrl("qrc:/webengineview.qml"));
#endif
                strVideoUrl = tmp_str;
                qInfo() << "strVideoUrl set to: " << strVideoUrl;
                // tell the QML side that value is updated
                emit videoUrlChanged();
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
    butConnect->setText ( tr ( "Join..." ) );

    // disable chat widgets
    butSend->setEnabled(false);
    edtLocalInputText->setEnabled(false);

    // reset Rec label (if server ends session)
    recLabel->setStyleSheet ( "QLabel { color: rgb(86, 86, 86); font: normal; }" );

    // reset session status bar
    sessionStatusLabel->setText("NO SESSION");
    sessionStatusLabel->setStyleSheet ( "QLabel { color: white; font: normal; }" );
    // reset server name in audio mixer group box title
//    MainMixerBoard->SetServerName ( "" );

    // Reset video view to No Session
//FIXME - we do special stuff for Android, because calling setSource() again causes webview to go FULL SCREEN
#if defined(ANDROID)
    strVideoUrl = "https://koord.live/about";
    emit videoUrlChanged();
    strVideoUrl = "";
    emit videoUrlChanged();
#else
    quickView->setSource(QUrl("qrc:/nosessionview.qml"));
#endif

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

    // clang-format off
// TODO is this still required???
// immediately update status bar
OnTimerStatus();
    // clang-format on

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
    QUrl url("https://api.github.com/repos/koord-live/koord-realtime/releases");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // send request and assign reply pointer
    QNetworkReply *reply = qNam->get(request);

    // connect reply pointer with callback for finished signal
    QObject::connect(reply, &QNetworkReply::finished, this, [=]()
        {
            QString err = reply->errorString();
            QString contents = QString::fromUtf8(reply->readAll());
            if ( reply->error() != QNetworkReply::NoError)
            {
                QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "Network Error!" );
                return;
            }
            //qInfo() << ">>> CONNECT: " << reply->error();

            QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
            QJsonArray jsonArray = jsonResponse.array();

            for (int jsIndex = 0; jsIndex < jsonArray.size(); ++jsIndex) {
                QJsonObject jsObject = jsonArray[jsIndex].toObject();
                foreach(const QString& key, jsObject.keys()) {
                    if ( key == "tag_name")
                    {
                        QJsonValue value = jsObject.value(key);
                        //qInfo() << "Key = " << key << ", Value = " << value.toString();
                        QRegularExpression rx_release("^r[0-9]+_[0-9]+_[0-9]+$");
                        QRegularExpressionMatch rel_match = rx_release.match(value.toString());
                        if (rel_match.hasMatch()) {
                            QString latestVersion =  rel_match.captured(0)
                                                       .replace("r", "")
                                                       .replace("_", ".");
                            if ( APP_VERSION == latestVersion )
                            {
                                //qInfo() << "WE HAVE A MATCH - no update necessary";
                                QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "Up to date!" );
                            }
                            else
                            {
                                //qInfo() << "Later version available: " << latestVersion;
                                downloadLinkButton->setVisible(true);
                                QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "Update Available!" );
                                downloadLinkButton->setText(QString("Download Version %1").arg(latestVersion));
                            };
                            // IF we have a match, then that is the latest version
                            // Github returns array with latest releases at start of index
                            // So return after first successful match
                            return;
                        };
                    }
                }
            }
            QToolTip::showText( checkUpdateButton->mapToGlobal( QPoint( 0, 0 ) ), "No Update Found" );
        });

}

void CClientDlg::UpdateDisplay()
{
//    // update settings/chat buttons (do not fire signals since it is an update)
//    if ( chbSettings->isChecked() && !ClientSettingsDlg.isVisible() )
//    {
//        chbSettings->blockSignals ( true );
//        chbSettings->setChecked ( false );
//        chbSettings->blockSignals ( false );
//    }
//    if ( !chbSettings->isChecked() && ClientSettingsDlg.isVisible() )
//    {
//        chbSettings->blockSignals ( true );
//        chbSettings->setChecked ( true );
//        chbSettings->blockSignals ( false );
//    }

//    if ( chbChat->isChecked() && !ChatDlg.isVisible() )
//    {
//        chbChat->blockSignals ( true );
//        chbChat->setChecked ( false );
//        chbChat->blockSignals ( false );
//    }
//    if ( !chbChat->isChecked() && ChatDlg.isVisible() )
//    {
//        chbChat->blockSignals ( true );
//        chbChat->setChecked ( true );
//        chbChat->blockSignals ( false );
//    }

//    if ( chbPubJam->isChecked() && !ConnectDlg.isVisible() )
//    {
//        chbPubJam->blockSignals ( true );
//        chbPubJam->setChecked ( false );
//        chbPubJam->blockSignals ( false );
//    }
//    if ( !chbPubJam->isChecked() && ConnectDlg.isVisible() )
//    {
//        chbPubJam->blockSignals ( true );
//        chbPubJam->setChecked ( true );
//        chbPubJam->blockSignals ( false );
//    }
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

// ======================================================================================
// SETTINGS - RELATED FUNCTIONS =========================================================
// ======================================================================================
void CClientDlg::showEvent ( QShowEvent* )
{
    UpdateDisplay();
    UpdateDirectoryServerComboBox();

    // set the name
    pedtAlias->setText ( pClient->ChannelInfo.strName );

    // select current instrument
    pcbxInstrument->setCurrentIndex ( pcbxInstrument->findData ( pClient->ChannelInfo.iInstrument ) );

    // select current country
    pcbxCountry->setCurrentIndex ( pcbxCountry->findData ( static_cast<int> ( pClient->ChannelInfo.eCountry ) ) );

    // set the city
    pedtCity->setText ( pClient->ChannelInfo.strCity );

    // select the skill level
    pcbxSkill->setCurrentIndex ( pcbxSkill->findData ( static_cast<int> ( pClient->ChannelInfo.eSkillLevel ) ) );
}

void CClientDlg::UpdateJitterBufferFrame()
{
    // update slider value and text
    const int iCurNumNetBuf = pClient->GetSockBufNumFrames();
    sldNetBuf->setValue ( iCurNumNetBuf );
    lblNetBuf->setText ( tr ( "Size: " ) + QString::number ( iCurNumNetBuf ) );

    const int iCurNumNetBufServer = pClient->GetServerSockBufNumFrames();
    sldNetBufServer->setValue ( iCurNumNetBufServer );
    lblNetBufServer->setText ( tr ( "Size: " ) + QString::number ( iCurNumNetBufServer ) );

    // if auto setting is enabled, disable slider control
    const bool bIsAutoSockBufSize = pClient->GetDoAutoSockBufSize();

    chbAutoJitBuf->setChecked ( bIsAutoSockBufSize );
    sldNetBuf->setEnabled ( !bIsAutoSockBufSize );
    lblNetBuf->setEnabled ( !bIsAutoSockBufSize );
    lblNetBufLabel->setEnabled ( !bIsAutoSockBufSize );
    sldNetBufServer->setEnabled ( !bIsAutoSockBufSize );
    lblNetBufServer->setEnabled ( !bIsAutoSockBufSize );
    lblNetBufServerLabel->setEnabled ( !bIsAutoSockBufSize );
}

QString CClientDlg::GenSndCrdBufferDelayString ( const int iFrameSize, const QString strAddText )
{
    // use two times the buffer delay for the entire delay since
    // we have input and output
    return QString().setNum ( static_cast<double> ( iFrameSize ) * 2 * 1000 / SYSTEM_SAMPLE_RATE_HZ, 'f', 2 ) + " ms (" +
           QString().setNum ( iFrameSize ) + strAddText + ")";
}

void CClientDlg::UpdateSoundCardFrame()
{
    // get current actual buffer size value
    const int iCurActualBufSize = pClient->GetSndCrdActualMonoBlSize();

    // check which predefined size is used (it is possible that none is used)
    const bool bPreferredChecked = ( iCurActualBufSize == SYSTEM_FRAME_SIZE_SAMPLES * FRAME_SIZE_FACTOR_PREFERRED );
    const bool bDefaultChecked   = ( iCurActualBufSize == SYSTEM_FRAME_SIZE_SAMPLES * FRAME_SIZE_FACTOR_DEFAULT );
    const bool bSafeChecked      = ( iCurActualBufSize == SYSTEM_FRAME_SIZE_SAMPLES * FRAME_SIZE_FACTOR_SAFE );

    // Set radio buttons according to current value (To make it possible
    // to have all radio buttons unchecked, we have to disable the
    // exclusive check for the radio button group. We require all radio
    // buttons to be unchecked in the case when the sound card does not
    // support any of the buffer sizes and therefore all radio buttons
    // are disabeld and unchecked.).
    SndCrdBufferDelayButtonGroup.setExclusive ( false );
    rbtBufferDelayPreferred->setChecked ( bPreferredChecked );
    rbtBufferDelayDefault->setChecked ( bDefaultChecked );
    rbtBufferDelaySafe->setChecked ( bSafeChecked );
    SndCrdBufferDelayButtonGroup.setExclusive ( true );

    // disable radio buttons which are not supported by audio interface
    rbtBufferDelayPreferred->setEnabled ( pClient->GetFraSiFactPrefSupported() );
    rbtBufferDelayDefault->setEnabled ( pClient->GetFraSiFactDefSupported() );
    rbtBufferDelaySafe->setEnabled ( pClient->GetFraSiFactSafeSupported() );

    // If any of our predefined sizes is chosen, use the regular group box
    // title text. If not, show the actual buffer size. Otherwise the user
    // would not know which buffer size is actually used.
    if ( bPreferredChecked || bDefaultChecked || bSafeChecked )
    {
        // default title text
        grbSoundCrdBufDelay->setTitle ( tr ( "Buffer Delay" ) );
    }
    else
    {
        // special title text with buffer size information added
        grbSoundCrdBufDelay->setTitle ( tr ( "Buffer Delay: " ) + GenSndCrdBufferDelayString ( iCurActualBufSize ) );
    }
}

void CClientDlg::UpdateSoundDeviceChannelSelectionFrame()
{
    // update combo box containing all available sound cards in the system
    QStringList slSndCrdDevNames = pClient->GetSndCrdDevNames();
    cbxSoundcard->clear();

    foreach ( QString strDevName, slSndCrdDevNames )
    {
        cbxSoundcard->addItem ( strDevName );
    }

    cbxSoundcard->setCurrentText ( pClient->GetSndCrdDev() );

    // update input/output channel selection
#if defined( _WIN32 ) || defined( __APPLE__ ) || defined( __MACOSX )

    // Definition: The channel selection frame shall only be visible,
    // if more than two input or output channels are available
    const int iNumInChannels  = pClient->GetSndCrdNumInputChannels();
    const int iNumOutChannels = pClient->GetSndCrdNumOutputChannels();

    if ( ( iNumInChannels < MIN_IN_CHANNELS ) || ( iNumOutChannels < MIN_OUT_CHANNELS ) )
    {
        // as defined, make settings invisible
        FrameSoundcardChannelSelection->setVisible ( false );
    }
    else
    {
        // update combo boxes
        FrameSoundcardChannelSelection->setVisible ( true );

        // input
        cbxLInChan->clear();
        cbxRInChan->clear();

        for ( int iSndChanIdx = 0; iSndChanIdx < pClient->GetSndCrdNumInputChannels(); iSndChanIdx++ )
        {
            cbxLInChan->addItem ( pClient->GetSndCrdInputChannelName ( iSndChanIdx ) );
            cbxRInChan->addItem ( pClient->GetSndCrdInputChannelName ( iSndChanIdx ) );
        }
        if ( pClient->GetSndCrdNumInputChannels() > 0 )
        {
            cbxLInChan->setCurrentIndex ( pClient->GetSndCrdLeftInputChannel() );
            cbxRInChan->setCurrentIndex ( pClient->GetSndCrdRightInputChannel() );
        }

        // output
        cbxLOutChan->clear();
        cbxROutChan->clear();
        for ( int iSndChanIdx = 0; iSndChanIdx < pClient->GetSndCrdNumOutputChannels(); iSndChanIdx++ )
        {
            cbxLOutChan->addItem ( pClient->GetSndCrdOutputChannelName ( iSndChanIdx ) );
            cbxROutChan->addItem ( pClient->GetSndCrdOutputChannelName ( iSndChanIdx ) );
        }
        if ( pClient->GetSndCrdNumOutputChannels() > 0 )
        {
            cbxLOutChan->setCurrentIndex ( pClient->GetSndCrdLeftOutputChannel() );
            cbxROutChan->setCurrentIndex ( pClient->GetSndCrdRightOutputChannel() );
        }
    }
#else
    // for other OS, no sound card channel selection is supported
    FrameSoundcardChannelSelection->setVisible ( false );
#endif
}

void CClientDlg::SetEnableFeedbackDetection ( bool enable )
{
    pSettings->bEnableFeedbackDetection = enable;
    chbDetectFeedback->setCheckState ( pSettings->bEnableFeedbackDetection ? Qt::Checked : Qt::Unchecked );
}

#if defined( _WIN32 ) && !defined( WITH_JACK )
void CClientDlg::OnDriverSetupClicked() { pClient->OpenSndCrdDriverSetup(); }
#endif

void CClientDlg::OnNetBufValueChanged ( int value )
{
    pClient->SetSockBufNumFrames ( value, true );
    UpdateJitterBufferFrame();
}

void CClientDlg::OnNetBufServerValueChanged ( int value )
{
    pClient->SetServerSockBufNumFrames ( value );
    UpdateJitterBufferFrame();
}

void CClientDlg::OnSoundcardActivated ( int iSndDevIdx )
{
    pClient->SetSndCrdDev ( cbxSoundcard->itemText ( iSndDevIdx ) );

    UpdateSoundDeviceChannelSelectionFrame();
    UpdateDisplay();
}

void CClientDlg::OnLInChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdLeftInputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientDlg::OnRInChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdRightInputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientDlg::OnLOutChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdLeftOutputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientDlg::OnROutChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdRightOutputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientDlg::OnAudioChannelsActivated ( int iChanIdx )
{
    pClient->SetAudioChannels ( static_cast<EAudChanConf> ( iChanIdx ) );
    emit AudioChannelsChanged();
    UpdateDisplay(); // upload rate will be changed
}

void CClientDlg::OnAudioQualityActivated ( int iQualityIdx )
{
    pClient->SetAudioQuality ( static_cast<EAudioQuality> ( iQualityIdx ) );
    UpdateDisplay(); // upload rate will be changed
}

void CClientDlg::OnGUIDesignActivated ( int iDesignIdx )
{
    pClient->SetGUIDesign ( static_cast<EGUIDesign> ( iDesignIdx ) );
    emit GUIDesignChanged();
    UpdateDisplay();
}

void CClientDlg::OnMeterStyleActivated ( int iMeterStyleIdx )
{
    pClient->SetMeterStyle ( static_cast<EMeterStyle> ( iMeterStyleIdx ) );
    emit MeterStyleChanged();
    UpdateDisplay();
}

void CClientDlg::OnAutoJitBufStateChanged ( int value )
{
    pClient->SetDoAutoSockBufSize ( value == Qt::Checked );
    UpdateJitterBufferFrame();
}

void CClientDlg::OnEnableOPUS64StateChanged ( int value )
{
    pClient->SetEnableOPUS64 ( value == Qt::Checked );
    UpdateDisplay();
}

void CClientDlg::OnFeedbackDetectionChanged ( int value ) { pSettings->bEnableFeedbackDetection = value == Qt::Checked; }

void CClientDlg::OnCustomDirectoriesEditingFinished()
{
    if ( cbxCustomDirectories->currentText().isEmpty() && cbxCustomDirectories->currentData().isValid() )
    {
        // if the user has selected an entry in the combo box list and deleted the text in the input field,
        // and then focus moves off the control without selecting a new entry,
        // we delete the corresponding entry in the vector
        pSettings->vstrDirectoryAddress[cbxCustomDirectories->currentData().toInt()] = "";
    }
    else if ( cbxCustomDirectories->currentData().isValid() && pSettings->vstrDirectoryAddress[cbxCustomDirectories->currentData().toInt()].compare (
                                                                   NetworkUtil::FixAddress ( cbxCustomDirectories->currentText() ) ) == 0 )
    {
        // if the user has selected another entry in the combo box list without changing anything,
        // there is no need to update any list
        return;
    }
    else
    {
        // store new address at the top of the list, if the list was already
        // full, the last element is thrown out
        pSettings->vstrDirectoryAddress.StringFiFoWithCompare ( NetworkUtil::FixAddress ( cbxCustomDirectories->currentText() ) );
    }

    // update combo box list and inform connect dialog about the new address
    UpdateDirectoryServerComboBox();
    emit CustomDirectoriesChanged();
}

void CClientDlg::OnSndCrdBufferDelayButtonGroupClicked ( QAbstractButton* button )
{
    if ( button == rbtBufferDelayPreferred )
    {
        pClient->SetSndCrdPrefFrameSizeFactor ( FRAME_SIZE_FACTOR_PREFERRED );
    }

    if ( button == rbtBufferDelayDefault )
    {
        pClient->SetSndCrdPrefFrameSizeFactor ( FRAME_SIZE_FACTOR_DEFAULT );
    }

    if ( button == rbtBufferDelaySafe )
    {
        pClient->SetSndCrdPrefFrameSizeFactor ( FRAME_SIZE_FACTOR_SAFE );
    }

    UpdateDisplay();
}

void CClientDlg::UpdateUploadRate()
{
    // update upstream rate information label
    lblUpstreamValue->setText ( QString().setNum ( pClient->GetUploadRateKbps() ) );
    lblUpstreamUnit->setText ( "kbps" );
}

void CClientDlg::UpdateSettingsDisplay()
{
    // update slider controls (settings might have been changed)
    UpdateJitterBufferFrame();
    UpdateSoundCardFrame();

    if ( !pClient->IsRunning() )
    {
        // clear text labels with client parameters
        lblUpstreamValue->setText ( "---" );
        lblUpstreamUnit->setText ( "" );
    }
}

void CClientDlg::UpdateDirectoryServerComboBox()
{
    cbxCustomDirectories->clear();
    cbxCustomDirectories->clearEditText();

    for ( int iLEIdx = 0; iLEIdx < MAX_NUM_SERVER_ADDR_ITEMS; iLEIdx++ )
    {
        if ( !pSettings->vstrDirectoryAddress[iLEIdx].isEmpty() )
        {
            // store the index as user data to the combo box item, too
            cbxCustomDirectories->addItem ( pSettings->vstrDirectoryAddress[iLEIdx], iLEIdx );
        }
    }
}

void CClientDlg::OnInputBoostChanged()
{
    // index is zero-based while boost factor must be 1-based:
//    pSettings->iInputBoost = cbxInputBoost->currentIndex() + 1;
    pSettings->iInputBoost = dialInputBoost->value();
    pClient->SetInputBoost ( pSettings->iInputBoost );
}

void CClientDlg::OnNewClientLevelChanged()
{
    // index is zero-based while boost factor must be 1-based:
//    pSettings->iInputBoost = cbxInputBoost->currentIndex() + 1;
    pSettings->iInputBoost = dialInputBoost->value();
    pClient->SetInputBoost ( pSettings->iInputBoost );

    pSettings->iNewClientFaderLevel = newInputLevelDial->value();
    edtNewClientLevel->setText(QString::number(newInputLevelDial->value()));
    //edtNewClientLevel->text().toInt();
}

void CClientDlg::OnAliasTextChanged ( const QString& strNewName )
{
    // check length
    if ( strNewName.length() <= MAX_LEN_FADER_TAG )
    {
        // refresh internal name parameter
        pClient->ChannelInfo.strName = strNewName;

        // update channel info at the server
        pClient->SetRemoteInfo();
    }
    else
    {
        // text is too long, update control with shortened text
        pedtAlias->setText ( TruncateString ( strNewName, MAX_LEN_FADER_TAG ) );
    }
}

void CClientDlg::OnInstrumentActivated ( int iCntryListItem )
{
    // set the new value in the data base
    pClient->ChannelInfo.iInstrument = pcbxInstrument->itemData ( iCntryListItem ).toInt();

    // update channel info at the server
    pClient->SetRemoteInfo();
}

void CClientDlg::OnCountryActivated ( int iCntryListItem )
{
    // set the new value in the data base
    pClient->ChannelInfo.eCountry = static_cast<QLocale::Country> ( pcbxCountry->itemData ( iCntryListItem ).toInt() );

    // update channel info at the server
    pClient->SetRemoteInfo();
}

void CClientDlg::OnCityTextChanged ( const QString& strNewCity )
{
    // check length
    if ( strNewCity.length() <= MAX_LEN_SERVER_CITY )
    {
        // refresh internal name parameter
        pClient->ChannelInfo.strCity = strNewCity;

        // update channel info at the server
        pClient->SetRemoteInfo();
    }
    else
    {
        // text is too long, update control with shortened text
        pedtCity->setText ( strNewCity.left ( MAX_LEN_SERVER_CITY ) );
    }
}

void CClientDlg::OnSkillActivated ( int iCntryListItem )
{
    // set the new value in the data base
    pClient->ChannelInfo.eSkillLevel = static_cast<ESkillLevel> ( pcbxSkill->itemData ( iCntryListItem ).toInt() );

    // update channel info at the server
    pClient->SetRemoteInfo();
}

//void CClientDlg::OnMakeTabChange ( int iTab )
//{
//    tabSettings->setCurrentIndex ( iTab );

//    pSettings->iSettingsTab = iTab;
//}

//void CClientDlg::OnTabChanged ( void ) { pSettings->iSettingsTab = tabSettings->currentIndex(); }

void CClientDlg::UpdateAudioFaderSlider()
{
    // update slider and label of audio fader
    const int iCurAudInFader = pClient->GetAudioInFader();
//    sldAudioPan->setValue ( iCurAudInFader );
    panDial->setValue ( iCurAudInFader );

    // show in label the center position and what channel is
    // attenuated
    if ( iCurAudInFader == AUD_FADER_IN_MIDDLE )
    {
        lblAudioPanValue->setText ( tr ( "Center" ) );
    }
    else
    {
        if ( iCurAudInFader > AUD_FADER_IN_MIDDLE )
        {
            // attenuation on right channel
            lblAudioPanValue->setText ( tr ( "L" ) + " -" + QString().setNum ( iCurAudInFader - AUD_FADER_IN_MIDDLE ) );
        }
        else
        {
            // attenuation on left channel
            lblAudioPanValue->setText ( tr ( "R" ) + " -" + QString().setNum ( AUD_FADER_IN_MIDDLE - iCurAudInFader ) );
        }
    }
}

void CClientDlg::OnAudioPanValueChanged ( int value )
{
    pClient->SetAudioInFader ( value );
    UpdateAudioFaderSlider();
}

// END SETTINGS FUNCTIONS ==========================================================================================

// QML FUNCTIONS
//QString CClientDlg::getVideoUrl()
//{
//    return strVideoUrl;
//}
