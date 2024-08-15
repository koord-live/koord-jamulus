#include "clientsettingsdlg.h"

/* Implementation *************************************************************/
CClientSettingsDlg::CClientSettingsDlg ( CClient* pNCliP, CClientSettings* pNSetP ) :
    // CBaseDlg ( parent, Qt::Window ), // use Qt::Window to get min/max window buttons
    pClient ( pNCliP ),
    pSettings ( pNSetP )
{
    // setupUi ( this );

    // transitional settings view
    settingsView = new QQuickView();
    // QWidget *settingsContainer = QWidget::createWindowContainer(settingsView, this);
    settingsView->setSource(QUrl("qrc:/settingview.qml"));
    // settingsTab->layout()->addWidget(settingsContainer);
    QQmlContext* settingsContext = settingsView->rootContext();

    settingsContext->setContextProperty("_settings", pSettings);

    // init audio in fader
//    sldAudioPan->setRange ( AUD_FADER_IN_MIN, AUD_FADER_IN_MAX );
    // panDial->setRange ( AUD_FADER_IN_MIN, AUD_FADER_IN_MAX );
//    sldAudioPan->setTickInterval ( AUD_FADER_IN_MAX / 5 );
    UpdateAudioFaderSlider();

    // init delay and other information controls
    // lblUpstreamValue->setText ( "---" );
    // lblUpstreamUnit->setText ( "" );
    // edtNewClientLevel->setValidator ( new QIntValidator ( 0, 100, this ) ); // % range from 0-100

    // init slider controls ---
    // network buffer sliders
    // sldNetBuf->setRange ( MIN_NET_BUF_SIZE_NUM_BL, MAX_NET_BUF_SIZE_NUM_BL );
    // sldNetBufServer->setRange ( MIN_NET_BUF_SIZE_NUM_BL, MAX_NET_BUF_SIZE_NUM_BL );
    UpdateJitterBufferFrame();

    // init sound card channel selection frame
    UpdateSoundDeviceChannelSelectionFrame();

    // // Audio Channels combo box
    // cbxAudioChannels->clear();
    // cbxAudioChannels->addItem ( tr ( "Mono" ) );               // CC_MONO
    // cbxAudioChannels->addItem ( tr ( "Mono-in/Stereo-out" ) ); // CC_MONO_IN_STEREO_OUT
    // cbxAudioChannels->addItem ( tr ( "Stereo" ) );             // CC_STEREO
    // cbxAudioChannels->setCurrentIndex ( static_cast<int> ( pClient->GetAudioChannels() ) );

    // Audio Quality combo box
    // cbxAudioQuality->clear();
    // cbxAudioQuality->addItem ( tr ( "Low" ) );    // AQ_LOW
    // cbxAudioQuality->addItem ( tr ( "Normal" ) ); // AQ_NORMAL
    // cbxAudioQuality->addItem ( tr ( "High" ) );   // AQ_HIGH
    // cbxAudioQuality->setCurrentIndex ( static_cast<int> ( pClient->GetAudioQuality() ) );

    // GUI design (skin) combo box
    // cbxSkin->clear();
    // cbxSkin->addItem ( tr ( "Normal" ) );  // GD_STANDARD
    // cbxSkin->addItem ( tr ( "Fancy" ) );   // GD_ORIGINAL
    // cbxSkin->addItem ( tr ( "Compact" ) ); // GD_SLIMFADER
    // cbxSkin->setCurrentIndex ( static_cast<int> ( pClient->GetGUIDesign() ) );

    // MeterStyle combo box
    // cbxMeterStyle->clear();
    // cbxMeterStyle->addItem ( tr ( "Bar (narrow)" ) );        // MT_BAR_NARROW
    // cbxMeterStyle->addItem ( tr ( "Bar (wide)" ) );          // MT_BAR_WIDE
    // cbxMeterStyle->addItem ( tr ( "LEDs (stripe)" ) );       // MT_LED_STRIPE
    // cbxMeterStyle->addItem ( tr ( "LEDs (round, small)" ) ); // MT_LED_ROUND_SMALL
    // cbxMeterStyle->addItem ( tr ( "LEDs (round, big)" ) );   // MT_LED_ROUND_BIG
    // cbxMeterStyle->setCurrentIndex ( static_cast<int> ( pClient->GetMeterStyle() ) );

    // language combo box (corrects the setting if language not found)
    // cbxLanguage->Init ( pSettings->strLanguage );

    // init custom directories combo box (max MAX_NUM_SERVER_ADDR_ITEMS entries)
    // cbxCustomDirectories->setMaxCount ( MAX_NUM_SERVER_ADDR_ITEMS );
    // cbxCustomDirectories->setInsertPolicy ( QComboBox::NoInsert );

    // update new client fader level edit box
    // edtNewClientLevel->setText ( QString::number ( pSettings->iNewClientFaderLevel ) );

//    // Input Boost combo box
//    cbxInputBoost->clear();
//    cbxInputBoost->addItem ( tr ( "None" ) );
//    for ( int i = 2; i <= 10; i++ )
//    {
//        cbxInputBoost->addItem ( QString ( "%1x" ).arg ( i ) );
//    }
    // factor is 1-based while index is 0-based:
//    cbxInputBoost->setCurrentIndex ( pSettings->iInputBoost - 1 );
    // dialInputBoost->setValue( pSettings->iInputBoost );

    // init number of mixer rows
    // spnMixerRows->setValue ( pSettings->iNumMixerPanelRows );

    // update feedback detection
    // chbDetectFeedback->setCheckState ( pSettings->bEnableFeedbackDetection ? Qt::Checked : Qt::Unchecked );

    // update enable small network buffers check box
    // chbEnableOPUS64->setCheckState ( pClient->GetEnableOPUS64() ? Qt::Checked : Qt::Unchecked );

    // set text for sound card buffer delay radio buttons
    // rbtBufferDelayPreferred->setText ( GenSndCrdBufferDelayString ( FRAME_SIZE_FACTOR_PREFERRED * SYSTEM_FRAME_SIZE_SAMPLES ) );

    // rbtBufferDelayDefault->setText ( GenSndCrdBufferDelayString ( FRAME_SIZE_FACTOR_DEFAULT * SYSTEM_FRAME_SIZE_SAMPLES ) );

    // rbtBufferDelaySafe->setText ( GenSndCrdBufferDelayString ( FRAME_SIZE_FACTOR_SAFE * SYSTEM_FRAME_SIZE_SAMPLES ) );

    // // sound card buffer delay inits
    // SndCrdBufferDelayButtonGroup.addButton ( rbtBufferDelayPreferred );
    // SndCrdBufferDelayButtonGroup.addButton ( rbtBufferDelayDefault );
    // SndCrdBufferDelayButtonGroup.addButton ( rbtBufferDelaySafe );

    UpdateSoundCardFrame();
#if defined ( Q_OS_WINDOWS )
    SetupBuiltinASIOBox();
#endif


    // ==================================================================================================
    // SETTINGS SLOTS ========
    // ==================================================================================================
    // Connections -------------------------------------------------------------
    // timers
    QObject::connect ( &TimerStatus, &QTimer::timeout, this, &CClientSettingsDlg::OnTimerStatus );

    // slider controls
    // QObject::connect ( sldNetBuf, &QSlider::valueChanged, this, &CClientSettingsDlg::OnNetBufValueChanged );

    // QObject::connect ( sldNetBufServer, &QSlider::valueChanged, this, &CClientSettingsDlg::OnNetBufServerValueChanged );

    // check boxes
    QObject::connect ( chbAutoJitBuf, &QCheckBox::stateChanged, this, &CClientSettingsDlg::OnAutoJitBufStateChanged );

    // QObject::connect ( chbEnableOPUS64, &QCheckBox::stateChanged, this, &CClientSettingsDlg::OnEnableOPUS64StateChanged );

    // QObject::connect ( chbDetectFeedback, &QCheckBox::stateChanged, this, &CClientSettingsDlg::OnFeedbackDetectionChanged );

    QObject::connect ( newInputLevelDial,
                       &QSlider::valueChanged,
                       this,
                       &CClientSettingsDlg::OnNewClientLevelChanged );

    // combo boxes
    QObject::connect ( cbxSoundcard,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnSoundcardActivated );
   
    QObject::connect ( cbxLInChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnLInChanActivated );

    QObject::connect ( cbxRInChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnRInChanActivated );

    QObject::connect ( cbxLOutChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnLOutChanActivated );

    QObject::connect ( cbxROutChan,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnROutChanActivated );

    QObject::connect ( cbxAudioChannels,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnAudioChannelsActivated );

    QObject::connect ( cbxAudioQuality,
                       static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
                       this,
                       &CClientSettingsDlg::OnAudioQualityActivated );

    // QObject::connect ( cbxSkin,
    //                    static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
    //                    this,
    //                    &CClientSettingsDlg::OnGUIDesignActivated );

    // QObject::connect ( cbxMeterStyle,
    //                    static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
    //                    this,
    //                    &CClientSettingsDlg::OnMeterStyleActivated );

    // QObject::connect ( cbxCustomDirectories->lineEdit(), &QLineEdit::editingFinished, this, &CClientSettingsDlg::OnCustomDirectoriesEditingFinished );

    // QObject::connect ( cbxCustomDirectories,
    //                    static_cast<void ( QComboBox::* ) ( int )> ( &QComboBox::activated ),
    //                    this,
    //                    &CClientSettingsDlg::OnCustomDirectoriesEditingFinished );

    // QObject::connect ( cbxLanguage, &CLanguageComboBox::LanguageChanged, this, &CClientSettingsDlg::OnLanguageChanged );

    // QObject::connect ( dialInputBoost,
    //                    &QSlider::valueChanged,
    //                    this,
    //                    &CClientSettingsDlg::OnInputBoostChanged );

    // buttons
#if defined( _WIN32 ) && !defined( WITH_JACK )
    // Driver Setup button is only available for Windows when JACK is not used
    QObject::connect ( butDriverSetup, &QPushButton::clicked, this, &CClientSettingsDlg::OnDriverSetupClicked );
    // make driver refresh button reload the currently selected sound card
//    QObject::connect ( driverRefresh, &QPushButton::clicked, this, &CClientSettingsDlg::OnSoundcardReactivate );
#endif

    // line edits
    QObject::connect ( edtNewClientLevel, &QLineEdit::editingFinished, this, &CClientSettingsDlg::OnNewClientLevelEditingFinished );

    // misc
    // sliders
    // panDial
    QObject::connect ( panDial, &QSlider::valueChanged, this, &CClientSettingsDlg::OnAudioPanValueChanged );

    QObject::connect ( &SndCrdBufferDelayButtonGroup,
                       static_cast<void ( QButtonGroup::* ) ( QAbstractButton* )> ( &QButtonGroup::buttonClicked ),
                       this,
                       &CClientSettingsDlg::OnSndCrdBufferDelayButtonGroupClicked );

    // spinners
    // QObject::connect ( spnMixerRows,
    //                    static_cast<void ( QSpinBox::* ) ( int )> ( &QSpinBox::valueChanged ),
    //                    this,
    //                    &CClientSettingsDlg::NumMixerPanelRowsChanged );

    // Musician Profile
    QObject::connect ( pedtAlias, &QLineEdit::textChanged, this, &CClientSettingsDlg::OnAliasTextChanged );


    // Timers ------------------------------------------------------------------
    // start timer for status bar
    TimerStatus.start ( DISPLAY_UPDATE_TIME );

    // END OF SETTINGS SLOTS =======================================================================================

}


// ======================================================================================
// SETTINGS - RELATED FUNCTIONS =========================================================
// ======================================================================================
void CClientSettingsDlg::showEvent ( QShowEvent* )
{
    UpdateDisplay();
    UpdateDirectoryServerComboBox();

           // get Region checker list
    RequestServerList();

           // set the name
    pedtAlias->setText ( pClient->ChannelInfo.strName );
}

void CClientSettingsDlg::UpdateJitterBufferFrame()
{
    // update slider value and text
    // const int iCurNumNetBuf = pClient->GetSockBufNumFrames();
    // sldNetBuf->setValue ( iCurNumNetBuf );
    // lblNetBuf->setText ( tr ( "Size: " ) + QString::number ( iCurNumNetBuf ) );

    // const int iCurNumNetBufServer = pClient->GetServerSockBufNumFrames();
    // sldNetBufServer->setValue ( iCurNumNetBufServer );
    // lblNetBufServer->setText ( tr ( "Size: " ) + QString::number ( iCurNumNetBufServer ) );

    // if auto setting is enabled, disableslider control
    const bool bIsAutoSockBufSize = pClient->GetDoAutoSockBufSize();

    chbAutoJitBuf->setChecked ( bIsAutoSockBufSize );
    sldNetBuf->setEnabled ( !bIsAutoSockBufSize );
    lblNetBuf->setEnabled ( !bIsAutoSockBufSize );
    lblNetBufLabel->setEnabled ( !bIsAutoSockBufSize );
    sldNetBufServer->setEnabled ( !bIsAutoSockBufSize );
    lblNetBufServer->setEnabled ( !bIsAutoSockBufSize );
    lblNetBufServerLabel->setEnabled ( !bIsAutoSockBufSize );
}

QString CClientSettingsDlg::GenSndCrdBufferDelayString ( const int iFrameSize, const QString strAddText )
{
    // use two times the buffer delay for the entire delay since
    // we have input and output
    return QString().setNum ( static_cast<double> ( iFrameSize ) * 2 * 1000 / SYSTEM_SAMPLE_RATE_HZ, 'f', 2 ) + " ms (" +
           QString().setNum ( iFrameSize ) + strAddText + ")";
}

void CClientSettingsDlg::UpdateSoundCardFrame()
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

void CClientSettingsDlg::UpdateSoundDeviceChannelSelectionFrame()
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
        GroupBoxSoundcardChannelSelection->setVisible ( false );
    }
    else
    {
        // update combo boxes
        GroupBoxSoundcardChannelSelection->setVisible ( true );

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
    GroupBoxSoundcardChannelSelection->setVisible ( false );
#endif
}

#if defined( Q_OS_WINDOWS )
void CClientSettingsDlg::SetupBuiltinASIOBox() {

    if ( cbxSoundcard->currentText() == "Built-in" ) {
        // show config box
        kdAsioGroupBox->show();
        // hide Driver Setup button - not needed
        driverSetupWidget->hide();
        // also hide buffer delay widget - it's confusing here
        grbSoundCrdBufDelay->hide();

       // multimedia stuff is only on Windows for now
#if defined( Q_OS_WINDOWS )
        // Simple test for USB devices - will typically contain string "usb" somewhere in device name
        if ( !inputDeviceName.contains("usb", Qt::CaseInsensitive) || !outputDeviceName.contains("usb", Qt::CaseInsensitive)) {
            qInfo() << "in_dev: " << inputDeviceName << " , out_dev: " << outputDeviceName;
            koordASIOWarningBox->show();
        } else {
            koordASIOWarningBox->hide();
        }
#endif
    } else {
        // hide config box
        kdAsioGroupBox->hide();
        // show Driver Setup button
        driverSetupWidget->show();
        // show buffer delay box
        grbSoundCrdBufDelay->show();
    }
}
#endif

// void CClientSettingsDlg::SetEnableFeedbackDetection ( bool enable )
// {
//     pSettings->bEnableFeedbackDetection = enable;
//     chbDetectFeedback->setCheckState ( pSettings->bEnableFeedbackDetection ? Qt::Checked : Qt::Unchecked );
// }

#if defined( _WIN32 ) && !defined( WITH_JACK )
void CClientSettingsDlg::OnDriverSetupClicked() { pClient->OpenSndCrdDriverSetup(); }
void CClientSettingsDlg::OnSoundcardReactivate() {
    qInfo() << "OnSoundcardReactivate(): activating card: " << cbxSoundcard->currentIndex();
    // simply set again the currently-set soundcard
    OnSoundcardActivated(cbxSoundcard->currentIndex());
}
#endif

void CClientSettingsDlg::OnNetBufValueChanged ( int value )
{
    pClient->SetSockBufNumFrames ( value, true );
    UpdateJitterBufferFrame();
}

void CClientSettingsDlg::OnNetBufServerValueChanged ( int value )
{
    pClient->SetServerSockBufNumFrames ( value );
    UpdateJitterBufferFrame();
}

void CClientSettingsDlg::OnSoundcardActivated ( int iSndDevIdx )
{
    qInfo() << "CCDlg:OnSoundcardActivated() ... setting sndcarddev: " << cbxSoundcard->itemText ( iSndDevIdx );
    pClient->SetSndCrdDev ( cbxSoundcard->itemText ( iSndDevIdx ) );

    UpdateSoundDeviceChannelSelectionFrame();
#if defined( Q_OS_WINDOWS )
    SetupBuiltinASIOBox();
#endif
    UpdateDisplay();
}

void CClientSettingsDlg::OnLInChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdLeftInputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientSettingsDlg::OnRInChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdRightInputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientSettingsDlg::OnLOutChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdLeftOutputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientSettingsDlg::OnROutChanActivated ( int iChanIdx )
{
    pClient->SetSndCrdRightOutputChannel ( iChanIdx );
    UpdateSoundDeviceChannelSelectionFrame();
}

void CClientSettingsDlg::OnAudioChannelsActivated ( int iChanIdx )
{
    pClient->SetAudioChannels ( static_cast<EAudChanConf> ( iChanIdx ) );
    emit AudioChannelsChanged();
    UpdateDisplay(); // upload rate will be changed
}

void CClientSettingsDlg::OnAudioQualityActivated ( int iQualityIdx )
{
    pClient->SetAudioQuality ( static_cast<EAudioQuality> ( iQualityIdx ) );
    UpdateDisplay(); // upload rate will be changed

}
void CClientSettingsDlg::OnGUIDesignActivated ( int iDesignIdx )
{
    pClient->SetGUIDesign ( static_cast<EGUIDesign> ( iDesignIdx ) );
    emit GUIDesignChanged();
    UpdateDisplay();
}

void CClientSettingsDlg::OnMeterStyleActivated ( int iMeterStyleIdx )
{
    pClient->SetMeterStyle ( static_cast<EMeterStyle> ( iMeterStyleIdx ) );
    emit MeterStyleChanged();
    UpdateDisplay();
}

void CClientSettingsDlg::OnAutoJitBufStateChanged ( int value )
{
    pClient->SetDoAutoSockBufSize ( value == Qt::Checked );
    UpdateJitterBufferFrame();
}

void CClientSettingsDlg::OnEnableOPUS64StateChanged ( int value )
{
    pClient->SetEnableOPUS64 ( value == Qt::Checked );
    UpdateDisplay();
}

void CClientSettingsDlg::OnFeedbackDetectionChanged ( int value ) { pSettings->bEnableFeedbackDetection = value == Qt::Checked; }

// void CClientSettingsDlg::OnCustomDirectoriesEditingFinished()
// {
//     if ( cbxCustomDirectories->currentText().isEmpty() && cbxCustomDirectories->currentData().isValid() )
//     {
//         // if the user has selected an entry in the combo box list and deleted the text in the input field,
//         // and then focus moves off the control without selecting a new entry,
//         // we delete the corresponding entry in the vector
//         pSettings->vstrDirectoryAddress[cbxCustomDirectories->currentData().toInt()] = "";
//     }
//     else if ( cbxCustomDirectories->currentData().isValid() && pSettings->vstrDirectoryAddress[cbxCustomDirectories->currentData().toInt()].compare (
//                                                                    NetworkUtil::FixAddress ( cbxCustomDirectories->currentText() ) ) == 0 )
//     {
//         // if the user has selected another entry in the combo box list without changing anything,
//         // there is no need to update any list
//         return;
//     }
//     else
//     {
//         // store new address at the top of the list, if the list was already
//         // full, the last element is thrown out
//         pSettings->vstrDirectoryAddress.StringFiFoWithCompare ( NetworkUtil::FixAddress ( cbxCustomDirectories->currentText() ) );
//     }

//            // update combo box list and inform connect dialog about the new address
//     UpdateDirectoryServerComboBox();
//     emit CustomDirectoriesChanged();
// }

// void CClientSettingsDlg::OnSndCrdBufferDelayButtonGroupClicked ( QAbstractButton* button )
// {
//     if ( button == rbtBufferDelayPreferred )
//     {
//         pClient->SetSndCrdPrefFrameSizeFactor ( FRAME_SIZE_FACTOR_PREFERRED );
//     }

//     if ( button == rbtBufferDelayDefault )
//     {
//         pClient->SetSndCrdPrefFrameSizeFactor ( FRAME_SIZE_FACTOR_DEFAULT );
//     }

//     if ( button == rbtBufferDelaySafe )
//     {
//         pClient->SetSndCrdPrefFrameSizeFactor ( FRAME_SIZE_FACTOR_SAFE );
//     }

//     UpdateDisplay();
// }

void CClientSettingsDlg::UpdateUploadRate()
{
    // update upstream rate information label
    lblUpstreamValue->setText ( QString().setNum ( pClient->GetUploadRateKbps() ) );
    lblUpstreamUnit->setText ( "kbps" );
}

void CClientSettingsDlg::UpdateSettingsDisplay()
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

//void CClientSettingsDlg::UpdateDirectoryServerComboBox()
//{
//    cbxCustomDirectories->clear();
//    cbxCustomDirectories->clearEditText();

//    for ( int iLEIdx = 0; iLEIdx < MAX_NUM_SERVER_ADDR_ITEMS; iLEIdx++ )
//    {
//        if ( !pSettings->vstrDirectoryAddress[iLEIdx].isEmpty() )
//        {
//            // store the index as user data to the combo box item, too
//            cbxCustomDirectories->addItem ( pSettings->vstrDirectoryAddress[iLEIdx], iLEIdx );
//        }
//    }
//}

// void CClientSettingsDlg::OnInputBoostChanged()
// {
//     // index is zero-based while boost factor must be 1-based:
//     //    pSettings->iInputBoost = cbxInputBoost->currentIndex() + 1;
//     pSettings->iInputBoost = dialInputBoost->value();
//     pClient->SetInputBoost ( pSettings->iInputBoost );
// }

// void CClientSettingsDlg::OnNewClientLevelChanged()
// {
//     // index is zero-based while boost factor must be 1-based:
//     //    pSettings->iInputBoost = cbxInputBoost->currentIndex() + 1;
//     pSettings->iInputBoost = dialInputBoost->value();
//     pClient->SetInputBoost ( pSettings->iInputBoost );

//     pSettings->iNewClientFaderLevel = newInputLevelDial->value();
//     edtNewClientLevel->setText(QString::number(newInputLevelDial->value()));
//     //edtNewClientLevel->text().toInt();
// }

void CClientSettingsDlg::OnAliasTextChanged ( const QString& strNewName )
{
    // check length
    if ( strNewName.length() <= MAX_LEN_FADER_TAG )
    {
        // refresh internal name parameter
        pClient->ChannelInfo.strName = strNewName;

               //FIXME - not working currently
        //        // reset videoUrl so that Jitsi/QML is updated
        //        strVideoUrl = strVideoHost + "#userInfo.displayName=\"" + pClient->ChannelInfo.strName + "\"";
        //        // tell the QML side that value is updated
        //        emit videoUrlChanged();

               // update channel info at the server
        pClient->SetRemoteInfo();
    }
    else
    {
        // text is too long, update control with shortened text
        pedtAlias->setText ( TruncateString ( strNewName, MAX_LEN_FADER_TAG ) );
    }

}


void CClientSettingsDlg::UpdateAudioFaderSlider()
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

void CClientSettingsDlg::OnAudioPanValueChanged ( int value )
{
    pClient->SetAudioInFader ( value );
    UpdateAudioFaderSlider();
    // wtf??
    UpdateAudioFaderSlider();
    UpdateAudioFaderSlider();
    UpdateAudioFaderSlider();
    UpdateAudioFaderSlider();
    UpdateAudioFaderSlider();
    UpdateAudioFaderSlider();
    UpdateAudioFaderSlider();
}

// END SETTINGS FUNCTIONS ==========================================================================================
