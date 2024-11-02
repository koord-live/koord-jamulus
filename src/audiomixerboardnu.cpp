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

#include "audiomixerboardnu.h"

/******************************************************************************\
* CChanneFader                                                                 *
\******************************************************************************/
CChannelFaderNu::CChannelFaderNu ( QObject* pNW ) :
    eDesign ( GD_STANDARD ),
    BitmapMutedIcon ( QString::fromUtf8 ( ":/png/fader/res/mutediconorange.png" ) ),
    bMIDICtrlUsed ( false )
{
    // create new GUI control objects and store pointers to them (note that
    // QWidget takes the ownership of the pMainGrid so that this only has
    // to be created locally in this constructor)
    // pFrame = new QFrame ( pNW );

    // pLevelsBox       = new QWidget ( pFrame );
    // plbrChannelLevel = new CLevelMeter ( pLevelsBox );
    // pFader           = new QSlider ( Qt::Vertical, pLevelsBox );
    // pPan             = new QDial ( pLevelsBox );
    // pPanLabel        = new QLabel ( tr ( "Pan" ), pLevelsBox );
    // pInfoLabel       = new QLabel ( "", pLevelsBox );

    // pMuteSoloBox = new QWidget ( pFrame );
    // pcbMute      = new QCheckBox ( tr ( "Mute" ), pMuteSoloBox );
    // pcbSolo      = new QCheckBox ( tr ( "Solo" ), pMuteSoloBox );
    // pcbGroup     = new QCheckBox ( "", pMuteSoloBox );

    // pLabelInstBox   = new QGroupBox ( pFrame );
    // plblLabel       = new QLabel ( "", pFrame );
    // plblInstrument  = new QLabel ( pFrame );
    // plblCountryFlag = new QLabel ( pFrame );

    // QVBoxLayout* pMainGrid     = new QVBoxLayout ( pFrame );
    // QHBoxLayout* pLevelsGrid   = new QHBoxLayout ( pLevelsBox );
    // QVBoxLayout* pMuteSoloGrid = new QVBoxLayout ( pMuteSoloBox );
    // pLabelGrid                 = new QHBoxLayout ( pLabelInstBox );
    // pLabelPictGrid             = new QVBoxLayout();
    // QVBoxLayout* pPanGrid      = new QVBoxLayout();
    // QHBoxLayout* pPanInfoGrid  = new QHBoxLayout();

    // define the popup menu for the group checkbox
    // pGroupPopupMenu = new QMenu ( "", pcbGroup );
    // pGroupPopupMenu->addAction ( tr ( "&No grouping" ), this, [=] { OnGroupMenuGrp ( INVALID_INDEX ); } );
    // for ( int iGrp = 0; iGrp < MAX_NUM_FADER_GROUPS; iGrp++ )
    // {
    //     pGroupPopupMenu->addAction ( tr ( "Assign to group" ) + ( QString ( " &%1" ).arg ( iGrp + 1 ) ), this, [=] { OnGroupMenuGrp ( iGrp ); } );
    // }
#if ( MAX_NUM_FADER_GROUPS != 8 )
#    error "MAX_NUM_FADER_GROUPS must be set to 8, see implementation in CChannelFaderNu()"
#endif

    // setup channel level
    // plbrChannelLevel->setContentsMargins ( 0, 3, 2, 3 );

    // // setup slider
    // pFader->setPageStep ( 1 );
    // pFader->setRange ( 0, AUD_MIX_FADER_MAX );
    // pFader->setTickInterval ( AUD_MIX_FADER_MAX / 9 );

    // // setup panning control and info label
    // pPan->setRange ( 0, AUD_MIX_PAN_MAX );
    // pPan->setValue ( AUD_MIX_PAN_MAX / 2 );
    // pPan->setNotchesVisible ( true );
    // pInfoLabel->setMinimumHeight ( 14 ); // prevents jitter when muting/unmuting (#811)
    // pInfoLabel->setAlignment ( Qt::AlignTop );
    // pPanInfoGrid->addWidget ( pPanLabel, 0, Qt::AlignLeft | Qt::AlignTop );
    // pPanInfoGrid->addWidget ( pInfoLabel, 0, Qt::AlignHCenter | Qt::AlignTop );
    // pPanGrid->addLayout ( pPanInfoGrid );
    // pPanGrid->addWidget ( pPan, 0, Qt::AlignHCenter );

    // // setup fader tag label (black bold text which is centered)
    // plblLabel->setTextFormat ( Qt::PlainText );
    // plblLabel->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );

    // // set margins of the layouts to zero to get maximum space for the controls
    // pMainGrid->setContentsMargins ( 0, 0, 0, 0 );

    // pPanGrid->setContentsMargins ( 0, 0, 0, 0 );
    // pPanGrid->setSpacing ( 0 ); // only minimal space

    // pLevelsGrid->setContentsMargins ( 0, 0, 0, 0 );
    // pLevelsGrid->setSpacing ( 0 ); // only minimal space

    // pMuteSoloGrid->setContentsMargins ( 0, 0, 0, 0 );
    // pMuteSoloGrid->setSpacing ( 0 ); // only minimal space

    // pLabelGrid->setContentsMargins ( 0, 0, 0, 0 );
    // pLabelGrid->setSpacing ( 2 ); // only minimal space between picture and text

    // // add user controls to the grids
    // // pLabelPictGrid->addWidget ( plblCountryFlag, 0, Qt::AlignHCenter );
    // // pLabelPictGrid->addWidget ( plblInstrument, 0, Qt::AlignHCenter );

    // pLabelGrid->addLayout ( pLabelPictGrid );
    // pLabelGrid->addWidget ( plblLabel, 0, Qt::AlignVCenter ); // note: just initial add, may be changed later

    // pLevelsGrid->addWidget ( plbrChannelLevel, 0, Qt::AlignRight );
    // pLevelsGrid->addWidget ( pFader, 0, Qt::AlignLeft );

    // pMuteSoloGrid->addWidget ( pcbGroup, 0, Qt::AlignLeft );
    // pMuteSoloGrid->addWidget ( pcbMute, 0, Qt::AlignLeft );
    // pMuteSoloGrid->addWidget ( pcbSolo, 0, Qt::AlignLeft );

    // pMainGrid->addLayout ( pPanGrid );
    // pMainGrid->addWidget ( pLevelsBox, 0, Qt::AlignHCenter );
    // pMainGrid->addWidget ( pMuteSoloBox, 0, Qt::AlignHCenter );
    // pMainGrid->addWidget ( pLabelInstBox );

    // reset current fader
    strGroupBaseText  = "Grp";         // this will most probably overwritten by SetGUIDesign()
    iInstrPicMaxWidth = INVALID_INDEX; // this will most probably overwritten by SetGUIDesign()
    Reset();

    // // add help text to controls
    // plbrChannelLevel->setWhatsThis ( "<b>" + tr ( "Channel Level" ) + ":</b> " +
    //                                  tr ( "Displays the pre-fader audio level of this channel.  All clients connected to the "
    //                                       "server will be assigned an audio level, the same value for every client." ) );
    // plbrChannelLevel->setAccessibleName ( tr ( "Input level of the current audio "
    //                                            "channel at the server" ) );

    // pFader->setWhatsThis ( "<b>" + tr ( "Mixer Fader" ) + ":</b> " +
    //                        tr ( "Adjusts the audio level of this channel. All clients connected to the server "
    //                             "will be assigned an audio fader, displayed at each client, to adjust the local mix." ) );
    // pFader->setAccessibleName ( tr ( "Local mix level setting of the current audio "
    //                                  "channel at the server" ) );

    // pInfoLabel->setWhatsThis ( "<b>" + tr ( "Status Indicator" ) + ":</b> " +
    //                            tr ( "Shows a status indication about the client which is assigned to this channel. "
    //                                 "Supported indicators are:" ) +
    //                            "<ul><li>" + tr ( "Speaker with cancellation stroke: Indicates that another client has muted you." ) + "</li></ul>" );
    // pInfoLabel->setAccessibleName ( tr ( "Status indicator label" ) );

    // pPan->setWhatsThis ( "<b>" + tr ( "Panning" ) + ":</b> " +
    //                      tr ( "Sets the pan from Left to Right of the channel. "
    //                           "Works only in stereo or preferably mono in/stereo out mode." ) );
    // pPan->setAccessibleName ( tr ( "Local panning position of the current audio channel at the server" ) );

    // pcbMute->setWhatsThis ( "<b>" + tr ( "Mute" ) + ":</b> " + tr ( "With the Mute checkbox, the audio channel can be muted." ) );
    // pcbMute->setAccessibleName ( tr ( "Mute button" ) );

    // pcbSolo->setWhatsThis ( "<b>" + tr ( "Solo" ) + ":</b> " +
    //                         tr ( "With the Solo checkbox, the "
    //                              "audio channel can be set to solo which means that all other channels "
    //                              "except the soloed channel are muted. It is possible to set more than "
    //                              "one channel to solo." ) );
    // pcbSolo->setAccessibleName ( tr ( "Solo button" ) );

    // pcbGroup->setWhatsThis ( "<b>" + tr ( "Group" ) + ":</b> " +
    //                          tr ( "With the Grp checkbox, a "
    //                               "group of audio channels can be defined. All channel faders in a group are moved "
    //                               "in proportional synchronization if any one of the group faders are moved." ) );
    // pcbGroup->setAccessibleName ( tr ( "Group button" ) );

    // QString strFaderText = "<b>" + tr ( "Fader Tag" ) + ":</b> " +
    //                        tr ( "The fader tag "
    //                             "identifies the connected client. The tag name, a picture of your "
    //                             "instrument and the flag of your location can be set in the main window." );

    // plblInstrument->setWhatsThis ( strFaderText );
    // plblInstrument->setAccessibleName ( tr ( "Mixer channel instrument picture" ) );
    // plblLabel->setWhatsThis ( strFaderText );
    // plblLabel->setAccessibleName ( tr ( "Mixer channel label (fader tag)" ) );
    // plblCountryFlag->setWhatsThis ( strFaderText );
    // plblCountryFlag->setAccessibleName ( tr ( "Mixer channel country/region flag" ) );

    // Connections -------------------------------------------------------------
    // QObject::connect ( pFader, &QSlider::valueChanged, this, &CChannelFaderNu::OnLevelValueChanged );

    // QObject::connect ( pPan, &QDial::valueChanged, this, &CChannelFaderNu::OnPanValueChanged );

    // QObject::connect ( pcbMute, &QCheckBox::stateChanged, this, &CChannelFaderNu::OnMuteStateChanged );

    // QObject::connect ( pcbSolo, &QCheckBox::stateChanged, this, &CChannelFaderNu::soloStateChanged );

    // QObject::connect ( pcbGroup, &QCheckBox::stateChanged, this, &CChannelFaderNu::OnGroupStateChanged );
}


void CChannelFaderNu::SetDisplayChannelLevel ( const bool eNDCL ) {} // plbrChannelLevel->setHidden ( !eNDCL ); }

bool CChannelFaderNu::GetDisplayChannelLevel() { return true; } //FIXME return !plbrChannelLevel->isHidden(); }

void CChannelFaderNu::SetDisplayPans ( const bool eNDP )
{
    // pPanLabel->setHidden ( !eNDP );
    // pPan->setHidden ( !eNDP );
}

void CChannelFaderNu::Reset()
{
    // it is important to reset the group index first (#611)
    iGroupID = INVALID_INDEX;

    // general initializations
    SetRemoteFaderIsMute ( false );

    // init gain and pan value -> maximum value as definition according to server
    // pFader->setValue ( AUD_MIX_FADER_MAX );
    dPreviousFaderLevel = AUD_MIX_FADER_MAX;
    // pPan->setValue ( AUD_MIX_PAN_MAX / 2 );

    // reset mute/solo/group check boxes and level meter
    // pcbMute->setChecked ( false );
    // pcbSolo->setChecked ( false );
    // plbrChannelLevel->SetValue ( 0 );
    // plbrChannelLevel->ClipReset();

    cReceivedChanInfo = CChannelInfo();
    // SetupFaderTag ( SL_NOT_SET );

    bOtherChannelIsSolo  = false;
    bIsMyOwnFader        = false;
    bIsMutedAtServer     = false;
    iRunningNewClientCnt = 0;

    //UpdateGroupIDDependencies();
}

void CChannelFaderNu::SetFaderLevel ( const double dLevel, const bool bIsGroupUpdate )
{
    // first make a range check
    if ( dLevel >= 0 )
    {
        // we set the new fader level in the GUI (slider control) and also tell the
        // server about the change (block the signal of the fader since we want to
        // call SendFaderLevelToServer with a special additional parameter)
        // pFader->blockSignals ( true );
        // pFader->setValue ( std::min ( AUD_MIX_FADER_MAX, MathUtils::round ( dLevel ) ) );
        // pFader->blockSignals ( false );

        SendFaderLevelToServer ( std::min ( static_cast<double> ( AUD_MIX_FADER_MAX ), dLevel ), bIsGroupUpdate );

        if ( dLevel > AUD_MIX_FADER_MAX )
        {
            // If the level is above the maximum, we have to store it for the purpose
            // of group fader movement. If you move a fader which has lower volume than
            // this one and this clips at max, we want to retain the ratio between this
            // fader and the others in the group.
            dPreviousFaderLevel = dLevel;
        }
    }
}

void CChannelFaderNu::SetPanValue ( const int iPan )
{
    // first make a range check
    if ( ( iPan >= 0 ) && ( iPan <= AUD_MIX_PAN_MAX ) )
    {
        // we set the new fader level in the GUI (slider control) which then
        // emits to signal to tell the server about the change (implicitly)
        // pPan->setValue ( iPan );
        // pPan->setAccessibleName ( QString::number ( iPan ) );
    }
}

void CChannelFaderNu::SetFaderIsSolo ( const bool bIsSolo )
{
    // changing the state automatically emits the signal, too
    // pcbSolo->setChecked ( bIsSolo );
}

void CChannelFaderNu::SetFaderIsMute ( const bool bIsMute )
{
    // changing the state automatically emits the signal, too
    // pcbMute->setChecked ( bIsMute );
}

void CChannelFaderNu::SetRemoteFaderIsMute ( const bool bIsMute )
{
    if ( bIsMute )
    {
        // show muted icon orange
        // pInfoLabel->setPixmap ( BitmapMutedIcon );
    }
    else
    {
        // pInfoLabel->setPixmap ( QPixmap() );
    }
}

void CChannelFaderNu::SendFaderLevelToServer ( const double dLevel, const bool bIsGroupUpdate )
{
    // if mute flag is set or other channel is on solo, do not apply the new
    // fader value to the server (exception: we are on solo, in that case we
    // ignore the "other channel is on solo" flag)

    // FIXME
    const bool bSuppressServerUpdate = true; // !( ( pcbMute->checkState() == Qt::Unchecked ) && ( !bOtherChannelIsSolo || IsSolo() ) );

    // emit signal for new fader gain value
    emit gainValueChanged ( MathUtils::CalcFaderGain ( static_cast<float> ( dLevel ) ),
                            bIsMyOwnFader,
                            bIsGroupUpdate,
                            bSuppressServerUpdate,
                            dLevel / dPreviousFaderLevel );

    // update previous fader level since the level has changed, avoid to use
    // the zero value not to have division by zero and also to retain the ratio
    // after the fader is moved up again from the zero position
    if ( dLevel > 0 )
    {
        dPreviousFaderLevel = dLevel;
    }
}

void CChannelFaderNu::SendPanValueToServer ( const int iPan ) { emit panValueChanged ( static_cast<float> ( iPan ) / AUD_MIX_PAN_MAX ); }

void CChannelFaderNu::OnPanValueChanged ( int value )
{
    // on shift-click the pan shall reset to 0 L/R (#707)
    if ( QGuiApplication::keyboardModifiers() == Qt::ShiftModifier )
    {
        // correct the value to the center position
        value = AUD_MIX_PAN_MAX / 2;

        // set the GUI control in the center position while deactivating
        // the signals to avoid an infinite loop
        // pPan->blockSignals ( true );
        // pPan->setValue ( value );
        // pPan->blockSignals ( false );
    }

    // pPan->setAccessibleName ( QString::number ( value ) );
    SendPanValueToServer ( value );
}

void CChannelFaderNu::OnMuteStateChanged ( int value )
{
    // call muting function
    SetMute ( static_cast<Qt::CheckState> ( value ) == Qt::Checked );
}

// void CChannelFaderNu::SetGroupID ( const int iNGroupID )
// {
//     iGroupID = iNGroupID;
//     UpdateGroupIDDependencies();
// }

// void CChannelFaderNu::UpdateGroupIDDependencies()
// {
//     // update the group checkbox according the current group ID setting
//     pcbGroup->blockSignals ( true ); // make sure no signals are fired
//     if ( iGroupID == INVALID_INDEX )
//     {
//         pcbGroup->setCheckState ( Qt::Unchecked );
//     }
//     else
//     {
//         pcbGroup->setCheckState ( Qt::Checked );
//     }
//     pcbGroup->blockSignals ( false );

//     // update group checkbox text
//     if ( iGroupID != INVALID_INDEX )
//     {
//         pcbGroup->setText ( strGroupBaseText + QString::number ( iGroupID + 1 ) );
//     }
//     else
//     {
//         pcbGroup->setText ( strGroupBaseText );
//     }

//     // if the group is disable for this fader, reset the previous fader level
//     if ( iGroupID == INVALID_INDEX )
//     {
//         // for the special case that the fader is all the way down, use a small
//         // value instead
//         if ( GetFaderLevel() > 0 )
//         {
//             dPreviousFaderLevel = GetFaderLevel();
//         }
//         else
//         {
//             dPreviousFaderLevel = 1; // small value
//         }
//     }

//     // the fader tag border color is set according to the selected group
//     SetupFaderTag ( cReceivedChanInfo.eSkillLevel );
// }

// void CChannelFaderNu::OnGroupStateChanged ( int )
// {
//     // we want a popup menu shown if the user presses the group checkbox but
//     // we want to make sure that the checkbox state represents the current group
//     // setting and not the current click state since the user might not click
//     // on the menu but at one other place and then the popup menu disappears but
//     // the checkobx state would be on an invalid state
//     UpdateGroupIDDependencies();
//     pGroupPopupMenu->popup ( QCursor::pos() );
// }

void CChannelFaderNu::SetMute ( const bool bState )
{
    if ( bState )
    {
        if ( !bIsMutedAtServer )
        {
            // mute channel -> send gain of 0
            emit gainValueChanged ( 0, bIsMyOwnFader, false, false, -1 ); // set level ratio to in invalid value
            bIsMutedAtServer = true;
        }
    }
    else
    {
        // only unmute if we are not solot but an other channel is on solo
        if ( ( !bOtherChannelIsSolo || IsSolo() ) && bIsMutedAtServer )
        {
            // mute was unchecked, get current fader value and apply
            emit gainValueChanged ( MathUtils::CalcFaderGain ( GetFaderLevel() ),
                                    bIsMyOwnFader,
                                    false,
                                    false,
                                    -1 ); // set level ratio to in invalid value
            bIsMutedAtServer = false;
        }
    }
}

void CChannelFaderNu::UpdateSoloState ( const bool bNewOtherSoloState )
{
    // store state (must be done before the SetMute() call!)
    bOtherChannelIsSolo = bNewOtherSoloState;

    // mute overwrites solo -> if mute is active, do not change anything
    // if ( !pcbMute->isChecked() )
    // {
    //     // mute channel if we are not solo but another channel is solo
    //     SetMute ( bOtherChannelIsSolo && !IsSolo() );
    // }
}

void CChannelFaderNu::SetChannelLevel ( const uint16_t iLevel ) {} // FIXME:  plbrChannelLevel->SetValue ( iLevel ); }

void CChannelFaderNu::SetChannelInfos ( const CChannelInfo& cChanInfo )
{
    // store received channel info
    cReceivedChanInfo = cChanInfo;

    // Label text --------------------------------------------------------------

    QString strModText = cChanInfo.strName;

    // show channel numbers if --ctrlmidich is used (#241, #95)
    if ( bMIDICtrlUsed )
    {
        strModText.prepend ( QString().setNum ( cChanInfo.iChanID ) + ":" );
    }
}

/******************************************************************************\
* CAudioMixerBoardNu                                                             *
\******************************************************************************/
CAudioMixerBoardNu::CAudioMixerBoardNu ( QObject* parent ) :
    // QGroupBox ( parent ),
    pSettings ( nullptr ),
    bDisplayPans ( false ),
    bIsPanSupported ( false ),
    bNoFaderVisible ( true ),
    iMyChannelID ( INVALID_INDEX ),
    iRunningNewClientCnt ( 0 ),
    iNumMixerPanelRows ( 1 ), // pSettings->iNumMixerPanelRows is not yet available
    strServerName ( "" ),
    eRecorderState ( RS_UNDEFINED ),
    eChSortType ( ST_NO_SORT )
{
    // add group box and hboxlayout
    // QHBoxLayout* pGroupBoxLayout = new QHBoxLayout ( this );
    // QWidget*     pMixerWidget    = new QWidget(); // will be added to the scroll area which is then the parent
    // pScrollArea                  = new CMixerBoardScrollArea ( this );
    // pMainLayout                  = new QGridLayout ( pMixerWidget );

    // set title text (default: no server given)
    SetServerName ( "" );

    // create all mixer controls and make them invisible
    vecpChanFader.Init ( MAX_NUM_CHANNELS );

    vecAvgLevels.Init ( MAX_NUM_CHANNELS, 0.0f );

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        // vecpChanFader[i] = new CChannelFaderNu ( this );
        vecpChanFader[i]->Hide();
    }

    // // insert horizontal spacer (at position MAX_NUM_CHANNELS+1 which is index MAX_NUM_CHANNELS)
    // pMainLayout->addItem ( new QSpacerItem ( 0, 0, QSizePolicy::Expanding ), 0, MAX_NUM_CHANNELS );

    // // set margins of the layout to zero to get maximum space for the controls
    // pGroupBoxLayout->setContentsMargins ( 0, 0, 0, 1 ); // note: to avoid problems at the bottom, use a small margin for that

    // // add the group box to the scroll area
    // pScrollArea->setMinimumWidth ( 200 ); // at least two faders shall be visible
    // pScrollArea->setWidget ( pMixerWidget );
    // pScrollArea->setWidgetResizable ( true ); // make sure it fills the entire scroll area
    // pScrollArea->setFrameShape ( QFrame::NoFrame );
    // pGroupBoxLayout->addWidget ( pScrollArea );

    // Connections -------------------------------------------------------------
    // connectFaderSignalsToMixerBoardSlots<MAX_NUM_CHANNELS>();
}

CAudioMixerBoardNu::~CAudioMixerBoardNu()
{
    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        delete vecpChanFader[i];
    }
}

// template<unsigned int slotId>
// inline void CAudioMixerBoardNu::connectFaderSignalsToMixerBoardSlots()
// {
//     size_t iCurChanID = slotId - 1;

//     void ( CAudioMixerBoardNu::*pGainValueChanged ) ( float, bool, bool, bool, double ) = &CAudioMixerBoardNuSlots<slotId>::OnChGainValueChanged;

//     void ( CAudioMixerBoardNu::*pPanValueChanged ) ( float ) = &CAudioMixerBoardNuSlots<slotId>::OnChPanValueChanged;

//     // QObject::connect ( vecpChanFader[iCurChanID], &CChannelFaderNu::soloStateChanged, this, &CAudioMixerBoardNu::UpdateSoloStates );

//     // QObject::connect ( vecpChanFader[iCurChanID], &CChannelFaderNu::gainValueChanged, this, pGainValueChanged );

//     // QObject::connect ( vecpChanFader[iCurChanID], &CChannelFaderNu::panValueChanged, this, pPanValueChanged );

//     connectFaderSignalsToMixerBoardSlots<slotId - 1>();
// }

// template<>
// inline void CAudioMixerBoardNu::connectFaderSignalsToMixerBoardSlots<0>()
// {}

void CAudioMixerBoardNu::SetServerName ( const QString& strNewServerName )
{
    // store the current server name
    strServerName = strNewServerName;

    if ( strServerName.isEmpty() )
    {
        // no connection or connection was reset: show default title
        //FIXME Don't want this hardcoding!
//        setTitle ( tr ( "Session Channels" ) );
        ;
        // don't do anything for now!
    }
    else
    {
        // Do not set the server name directly but first show a label which indicates
        // that we are trying to connect the server. First if a connected client
        // list was received, the connection was successful and the title is updated
        // with the correct server name. Make sure to choose a "try to connect" title
        // which is most striking (we use filled blocks and upper case letters).
//        setTitle ( u8"\u2588\u2588\u2588\u2588\u2588  " + tr ( "T R Y I N G   T O   C O N N E C T" ) + u8"  \u2588\u2588\u2588\u2588\u2588" );
        ;
        // don't do anything for now !!
    }
}

void CAudioMixerBoardNu::SetDisplayPans ( const bool eNDP )
{
    bDisplayPans = eNDP;

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        vecpChanFader[i]->SetDisplayPans ( eNDP && bIsPanSupported );
    }
}

void CAudioMixerBoardNu::SetPanIsSupported()
{
    bIsPanSupported = true;
    SetDisplayPans ( bDisplayPans );
}

void CAudioMixerBoardNu::HideAll()
{
    // before hiding the faders, store their settings
    StoreAllFaderSettings();

    // make all controls invisible
    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        vecpChanFader[i]->SetChannelLevel ( 0 );
        vecpChanFader[i]->SetDisplayChannelLevel ( false );
        vecpChanFader[i]->SetDisplayPans ( false );
        vecpChanFader[i]->Hide();
    }

    // initialize flags and other parameters
    bIsPanSupported      = false;
    bNoFaderVisible      = true;
    eRecorderState       = RS_UNDEFINED;
    iMyChannelID         = INVALID_INDEX;
    iRunningNewClientCnt = 0; // reset running counter on new server connection

    // use original order of channel (by server ID)
    ChangeFaderOrder ( ST_NO_SORT );

    // Reset recording indication styleSheet
    // setStyleSheet ( "" );

    // emit status of connected clients
    emit NumClientsChanged ( 0 ); // -> no clients connected
}

void CAudioMixerBoardNu::SetNumMixerPanelRows ( const int iNNumMixerPanelRows )
{
    // store new value and immediately initiate the sorting
    iNumMixerPanelRows = iNNumMixerPanelRows;
    ChangeFaderOrder ( eChSortType );
}

void CAudioMixerBoardNu::SetFaderSorting ( const EChSortType eNChSortType )
{
    // store new sort type and update the fader order
    eChSortType = eNChSortType;
    ChangeFaderOrder ( eNChSortType );
}

void CAudioMixerBoardNu::ChangeFaderOrder ( const EChSortType eChSortType )
{
    QMutexLocker locker ( &Mutex );

    // create a pair list of lower strings and fader ID for each channel
    QList<QPair<QString, size_t>> PairList;
    int                           iNumVisibleFaders = 0;
    int                           iMyFader          = -1;

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        if ( vecpChanFader[i]->GetIsMyOwnFader() )
        {
            iMyFader = static_cast<int> ( i );
        }

        if ( eChSortType == ST_BY_NAME )
        {
            PairList << QPair<QString, size_t> ( vecpChanFader[i]->GetReceivedName().toLower(), i );
        }
        // else if ( eChSortType == ST_BY_CITY )
        // {
        //     PairList << QPair<QString, size_t> ( vecpChanFader[i]->GetReceivedCity().toLower(), i );
        // }
        // else if ( eChSortType == ST_BY_INSTRUMENT )
        // {
        //     // sort first "by instrument" and second "by name" by adding the name after the instrument
        //     PairList << QPair<QString, size_t> ( CInstPictures::GetName ( vecpChanFader[i]->GetReceivedInstrument() ) +
        //                                              vecpChanFader[i]->GetReceivedName().toLower(),
        //                                          i );
        // }
        else if ( eChSortType == ST_BY_GROUPID )
        {
            if ( vecpChanFader[i]->GetGroupID() == INVALID_INDEX )
            {
                // put channels without a group at the end
                PairList << QPair<QString, size_t> ( "z", i ); // group IDs are numbers, use letter to put it at the end
            }
            else
            {
                PairList << QPair<QString, size_t> ( QString::number ( vecpChanFader[i]->GetGroupID() ), i );
            }
        }
        else // ST_NO_SORT
        {
            // per definition for no sort: faders are sorted in the order they appeared (note that we
            // pad to a total of 11 characters with zeros to make sure the sorting is done correctly)
            PairList << QPair<QString, size_t> ( QString ( "%1" ).arg ( vecpChanFader[i]->GetRunningNewClientCnt(), 11, 10, QLatin1Char ( '0' ) ),
                                                 i );
        }

        // count the number of visible faders
        if ( vecpChanFader[i]->IsVisible() )
        {
            iNumVisibleFaders++;
        }
    }

    // sort the channels according to the first of the pair
    std::stable_sort ( PairList.begin(), PairList.end() );

    // move my fader to first position
    if ( pSettings->bOwnFaderFirst )
    {
        for ( int i = 0; i < MAX_NUM_CHANNELS; i++ )
        {
            if ( iMyFader == static_cast<int> ( PairList[i].second ) )
            {
                PairList.move ( i, 0 );
                break;
            }
        }
    }

    // we want to distribute iNumVisibleFaders across the first row, then the next, etc
    // up to iNumMixerPanelRows.  So row wants to start at 0 until we get to some number,
    // then increase, where "some number" means we get no more than iNumMixerPanelRows.
    const int iNumFadersFirstRow = ( iNumVisibleFaders + iNumMixerPanelRows - 1 ) / iNumMixerPanelRows;

    // add channels to the layout in the new order, note that it is not required to remove
    // the widget from the layout first but it is moved to the new position automatically
    int iVisibleFaderCnt = 0;

    for ( int i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        const size_t iCurFaderID = PairList[i].second;

        if ( vecpChanFader[iCurFaderID]->IsVisible() )
        {
            // channels are added row-first, up to iNumFadersFirstRow, then onto
            // the next row.
            // pMainLayout->addWidget ( vecpChanFader[iCurFaderID]->GetMainWidget(),
            //                          iVisibleFaderCnt / iNumFadersFirstRow,
            //                          iVisibleFaderCnt % iNumFadersFirstRow );

            iVisibleFaderCnt++;
        }
    }
}

void CAudioMixerBoardNu::UpdateTitle()
{
    QString strTitlePrefix = "";

    if ( eRecorderState == RS_RECORDING )
    {
//        strTitlePrefix = "[" + tr ( "RECORDING ACTIVE" ) + "] ";
        // nope don't set title here any more
    }

    // replace & signs with && (See Qt documentation for QLabel)
    // if strServerName includes an "&" sign, this is interpreted as keyboard shortcut (#1886)
    // it might be possible to find a more elegant solution here?

    QString strEscServerName = strServerName;
    strEscServerName.replace ( "&", "&&" );

    //FIXME let's not do hard-coding like this
//    setTitle ( strTitlePrefix + tr ( "SESSION Mix at: " ) + strEscServerName );
    // don't set title here now!
    // setAccessibleName ( title() );
}

void CAudioMixerBoardNu::SetRecorderState ( const ERecorderState newRecorderState )
{
    // store the new recorder state and update the title
    eRecorderState = newRecorderState;
    UpdateTitle();
}

void CAudioMixerBoardNu::ApplyNewConClientList ( CVector<CChannelInfo>& vecChanInfo )
{
    // get number of connected clients
    const size_t iNumConnectedClients = vecChanInfo.size();

    Mutex.lock();
    {
        // we want to set the server name only if the very first faders appear
        // in the audio mixer board to show a "try to connect" before
        if ( bNoFaderVisible )
        {
            UpdateTitle();
        }

        // search for channels which are already present and preserve their gain
        // setting, for all other channels reset gain

        // get all channels which are in use/not in use.
        // We use the array index of vecChanInfo if the fader is in use,
        // else INVALID_INDEX to specify it is not in use
        // so must use "int" for the array type.
        int iFaderNumber[MAX_NUM_CHANNELS];

        for ( size_t iChanID = 0; iChanID < MAX_NUM_CHANNELS; iChanID++ )
        {
            iFaderNumber[iChanID] = INVALID_INDEX;
        }

        for ( size_t iFader = 0; iFader < iNumConnectedClients; iFader++ )
        {
            // ideally "iChanID" in CChannelInfo would be size_t if it can never be INVALID_INDEX
            // as assumed here
            iFaderNumber[vecChanInfo[iFader].iChanID] = static_cast<int> ( iFader );
        }

        // Hide all unused faders and initialize used ones
        for ( size_t iChanID = 0; iChanID < MAX_NUM_CHANNELS; iChanID++ )
        {
            if ( iFaderNumber[iChanID] == INVALID_INDEX )
            {
                // current fader is not used
                StoreFaderSettings ( vecpChanFader[iChanID] );

                vecpChanFader[iChanID]->Hide();
                continue;
            }
            size_t idxVecpChan = static_cast<size_t> ( iFaderNumber[iChanID] );

            // current fader is used
            if ( !vecpChanFader[iChanID]->IsVisible() )
            {
                // the fader was not in use,
                // reset everything for new client
                vecpChanFader[iChanID]->Reset();
                vecAvgLevels[iChanID] = 0.0f;

                if ( static_cast<int> ( iChanID ) == iMyChannelID )
                {
                    // this is my own fader --> set fader property
                    vecpChanFader[iChanID]->SetIsMyOwnFader();
                }

                // keep track of each new client
                // for "no sorting" channel sort order new clients are added
                // to the right-hand side of the mixer (#673)
                vecpChanFader[iChanID]->SetRunningNewClientCnt ( iRunningNewClientCnt++ );

                // show fader
                vecpChanFader[iChanID]->Show();

                // Set the default initial fader level. Check first that
                // this is not the initialization (i.e. previously there
                // were no faders visible) to avoid that our own level is
                // adjusted. If we have received our own channel ID, then
                // we can adjust the level even if no fader was visible.
                // The fader level of 100 % is the default in the
                // server, in that case we do not have to do anything here.
                if ( ( !bNoFaderVisible || ( ( iMyChannelID != INVALID_INDEX ) && ( iMyChannelID != static_cast<int> ( iChanID ) ) ) ) &&
                     ( pSettings->iNewClientFaderLevel != 100 ) )
                {
                    // the value is in percent -> convert range
                    vecpChanFader[iChanID]->SetFaderLevel ( pSettings->iNewClientFaderLevel / 100.0 * AUD_MIX_FADER_MAX );
                }
            }

            if ( vecpChanFader[iChanID]->GetReceivedName().compare ( vecChanInfo[idxVecpChan].strName ) )
            {
                // the text has actually changed, search in the list of
                // stored settings if we have a matching entry
                int  iStoredFaderLevel;
                int  iStoredPanValue;
                bool bStoredFaderIsSolo;
                bool bStoredFaderIsMute;
                int  iGroupID;

                if ( GetStoredFaderSettings ( vecChanInfo[idxVecpChan].strName,
                                              iStoredFaderLevel,
                                              iStoredPanValue,
                                              bStoredFaderIsSolo,
                                              bStoredFaderIsMute,
                                              iGroupID ) )
                {
                    vecpChanFader[iChanID]->SetFaderLevel ( iStoredFaderLevel, true ); // suppress group update
                    vecpChanFader[iChanID]->SetPanValue ( iStoredPanValue );
                    vecpChanFader[iChanID]->SetFaderIsSolo ( bStoredFaderIsSolo );
                    vecpChanFader[iChanID]->SetFaderIsMute ( bStoredFaderIsMute );
                    // vecpChanFader[iChanID]->SetGroupID ( iGroupID ); // Must be the last to be set in the fader!
                }
            }

            // set the channel infos
            // vecpChanFader[iChanID]->SetChannelInfos ( vecChanInfo[idxVecpChan] );
        }

        // update the solo states since if any channel was on solo and a new client
        // has just connected, the new channel must be muted
        UpdateSoloStates();

        // update flag for "all faders are invisible"
        bNoFaderVisible = ( iNumConnectedClients == 0 );
    }
    Mutex.unlock(); // release mutex

    // sort the channels according to the selected sorting type
    ChangeFaderOrder ( eChSortType );

    // emit status of connected clients
    emit NumClientsChanged ( static_cast<int> ( iNumConnectedClients ) );
}

void CAudioMixerBoardNu::SetFaderLevel ( const int iChannelIdx, const int iValue )
{
    // only apply new fader level if channel index is valid and the fader is visible
    if ( ( iChannelIdx >= 0 ) && ( iChannelIdx < MAX_NUM_CHANNELS ) )
    {
        if ( vecpChanFader[static_cast<size_t> ( iChannelIdx )]->IsVisible() )
        {
            vecpChanFader[static_cast<size_t> ( iChannelIdx )]->SetFaderLevel ( iValue );
        }
    }
}

void CAudioMixerBoardNu::SetPanValue ( const int iChannelIdx, const int iValue )
{
    // only apply new pan value if channel index is valid and the panner is visible
    if ( ( iChannelIdx >= 0 ) && ( iChannelIdx < MAX_NUM_CHANNELS ) && bDisplayPans )
    {
        if ( vecpChanFader[static_cast<size_t> ( iChannelIdx )]->IsVisible() )
        {
            vecpChanFader[static_cast<size_t> ( iChannelIdx )]->SetPanValue ( iValue );
        }
    }
}

void CAudioMixerBoardNu::SetFaderIsSolo ( const int iChannelIdx, const bool bIsSolo )
{
    // only apply solo if channel index is valid and the fader is visible
    if ( ( iChannelIdx >= 0 ) && ( iChannelIdx < MAX_NUM_CHANNELS ) )

    {
        if ( vecpChanFader[static_cast<size_t> ( iChannelIdx )]->IsVisible() )
        {
            vecpChanFader[static_cast<size_t> ( iChannelIdx )]->SetFaderIsSolo ( bIsSolo );
        }
    }
}

void CAudioMixerBoardNu::SetFaderIsMute ( const int iChannelIdx, const bool bIsMute )
{
    // only apply mute if channel index is valid and the fader is visible
    if ( ( iChannelIdx >= 0 ) && ( iChannelIdx < MAX_NUM_CHANNELS ) )
    {
        if ( vecpChanFader[static_cast<size_t> ( iChannelIdx )]->IsVisible() )
        {
            vecpChanFader[static_cast<size_t> ( iChannelIdx )]->SetFaderIsMute ( bIsMute );
        }
    }
}

void CAudioMixerBoardNu::SetAllFaderLevelsToNewClientLevel()
{
    QMutexLocker locker ( &Mutex );

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        // only apply to visible faders and not to my own channel fader
        if ( vecpChanFader[i]->IsVisible() && ( static_cast<int> ( i ) != iMyChannelID ) )
        {
            // the value is in percent -> convert range, also use the group
            // update flag to make sure the group values are all set to the
            // same fader level now
            vecpChanFader[i]->SetFaderLevel ( pSettings->iNewClientFaderLevel / 100.0 * AUD_MIX_FADER_MAX, true );
        }
    }
}

void CAudioMixerBoardNu::AutoAdjustAllFaderLevels()
{
    QMutexLocker locker ( &Mutex );

    // initialize variables used for statistics
    float vecMaxLevel[MAX_NUM_FADER_GROUPS + 1];
    int   vecChannelsPerGroup[MAX_NUM_FADER_GROUPS + 1];
    for ( int i = 0; i < MAX_NUM_FADER_GROUPS + 1; ++i )
    {
        vecMaxLevel[i]         = LOW_BOUND_SIG_METER;
        vecChannelsPerGroup[i] = 0;
    }
    CVector<CVector<float>> levels;
    levels.resize ( MAX_NUM_FADER_GROUPS + 1 );

    // compute min/max level per group and number of channels per group
    for ( size_t i = 0; i < MAX_NUM_CHANNELS; ++i )
    {
        // only apply to visible faders (and not to my own channel fader)
        if ( vecpChanFader[i]->IsVisible() && ( static_cast<int> ( i ) != iMyChannelID ) )
        {
            // map averaged meter output level to decibels
            // (invert CStereoSignalLevelMeter::CalcLogResultForMeter)
            float leveldB = vecAvgLevels[i] * ( UPPER_BOUND_SIG_METER - LOW_BOUND_SIG_METER ) / NUM_STEPS_LED_BAR + LOW_BOUND_SIG_METER;

            size_t group = MAX_NUM_FADER_GROUPS;
            if ( vecpChanFader[i]->GetGroupID() != INVALID_INDEX )
            {
                group = static_cast<size_t> ( vecpChanFader[i]->GetGroupID() );
            }

            if ( leveldB >= AUTO_FADER_NOISE_THRESHOLD_DB )
            {
                vecMaxLevel[group] = fmax ( vecMaxLevel[group], leveldB );
                levels[group].Add ( leveldB );
            }
            ++vecChannelsPerGroup[group];
        }
    }

    // sort levels for later median computation
    for ( size_t i = 0; i < MAX_NUM_FADER_GROUPS + 1; ++i )
    {
        std::sort ( levels[i].begin(), levels[i].end() );
    }

    // compute the number of active groups (at least one channel)
    int cntActiveGroups = 0;
    for ( int i = 0; i < MAX_NUM_FADER_GROUPS; ++i )
    {
        cntActiveGroups += vecChannelsPerGroup[i] > 0;
    }

    // only my channel is active, nothing to do
    if ( cntActiveGroups == 0 && vecChannelsPerGroup[MAX_NUM_FADER_GROUPS] == 0 )
    {
        return;
    }

    // compute target level for each group
    // (prevent clipping when each group contributes at maximum level)
    float targetLevelPerGroup = -20.0f * log10 ( std::max ( cntActiveGroups, 1 ) );

    // compute target levels for the channels of each group individually
    float vecTargetChannelLevel[MAX_NUM_FADER_GROUPS + 1];
    float levelOffset = 0.0f;
    float minFader    = 0.0f;
    for ( size_t i = 0; i < MAX_NUM_FADER_GROUPS + 1; ++i )
    {
        // compute the target level for each channel in the current group
        // (prevent clipping when each channel in this group contributes at
        // the maximum level)
        vecTargetChannelLevel[i] = vecChannelsPerGroup[i] > 0 ? targetLevelPerGroup - 20.0f * log10 ( vecChannelsPerGroup[i] ) : 0.0f;

        // get median level
        size_t cntChannels = levels[i].size();
        if ( cntChannels == 0 )
        {
            continue;
        }
        float refLevel = levels[i][cntChannels / 2];

        // since we can only attenuate channels but not amplify, we have to
        // check that the reference channel can be brought to the target
        // level
        if ( refLevel < vecTargetChannelLevel[i] )
        {
            // otherwise, we adjust the level offset in such a way that
            // the level can be reached
            levelOffset = fmin ( levelOffset, refLevel - vecTargetChannelLevel[i] );

            // compute the minimum necessary fader setting
            minFader = fmin ( minFader, -vecMaxLevel[i] + vecTargetChannelLevel[i] + levelOffset );
        }
    }

    // take minimum fader value into account
    // very weak channels would actually require strong channels to be
    // attenuated to a large amount; however, the attenuation is limited by
    // the faders
    if ( minFader < -AUD_MIX_FADER_RANGE_DB )
    {
        levelOffset += -AUD_MIX_FADER_RANGE_DB - minFader;
    }

    // adjust all levels
    for ( size_t i = 0; i < MAX_NUM_CHANNELS; ++i )
    {
        // only apply to visible faders (and not to my own channel fader)
        if ( vecpChanFader[i]->IsVisible() && ( static_cast<int> ( i ) != iMyChannelID ) )
        {
            // map averaged meter output level to decibels
            // (invert CStereoSignalLevelMeter::CalcLogResultForMeter)
            float leveldB = vecAvgLevels[i] * ( UPPER_BOUND_SIG_METER - LOW_BOUND_SIG_METER ) / NUM_STEPS_LED_BAR + LOW_BOUND_SIG_METER;

            int group = vecpChanFader[i]->GetGroupID();
            if ( group == INVALID_INDEX )
            {
                if ( cntActiveGroups > 0 )
                {
                    // do not adjust the channels without group in group mode
                    continue;
                }
                else
                {
                    group = MAX_NUM_FADER_GROUPS;
                }
            }

            // do not adjust channels with almost zero level to full level since
            // the channel might simply be silent at the moment
            if ( leveldB >= AUTO_FADER_NOISE_THRESHOLD_DB )
            {
                // compute new level
                float newdBLevel = -leveldB + vecTargetChannelLevel[group] + levelOffset;

                // map range from decibels to fader level
                // (this inverts MathUtils::CalcFaderGain)
                float newFaderLevel = ( newdBLevel / AUD_MIX_FADER_RANGE_DB + 1.0f ) * AUD_MIX_FADER_MAX;

                // limit fader
                newFaderLevel = fmin ( fmax ( newFaderLevel, 0.0f ), float ( AUD_MIX_FADER_MAX ) );

                // set fader level
                vecpChanFader[i]->SetFaderLevel ( newFaderLevel, true );
            }
        }
    }
}

void CAudioMixerBoardNu::SetMIDICtrlUsed ( const bool bMIDICtrlUsed )
{
    QMutexLocker locker ( &Mutex );

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        vecpChanFader[i]->SetMIDICtrlUsed ( bMIDICtrlUsed );
    }
}

void CAudioMixerBoardNu::StoreAllFaderSettings()
{
    QMutexLocker locker ( &Mutex );

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        StoreFaderSettings ( vecpChanFader[i] );
    }
}

void CAudioMixerBoardNu::LoadAllFaderSettings()
{
    QMutexLocker locker ( &Mutex );

    int  iStoredFaderLevel;
    int  iStoredPanValue;
    bool bStoredFaderIsSolo;
    bool bStoredFaderIsMute;
    int  iGroupID;

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        if ( GetStoredFaderSettings ( vecpChanFader[i]->GetReceivedName(),
                                      iStoredFaderLevel,
                                      iStoredPanValue,
                                      bStoredFaderIsSolo,
                                      bStoredFaderIsMute,
                                      iGroupID ) )
        {
            vecpChanFader[i]->SetFaderLevel ( iStoredFaderLevel, true ); // suppress group update
            vecpChanFader[i]->SetPanValue ( iStoredPanValue );
            vecpChanFader[i]->SetFaderIsSolo ( bStoredFaderIsSolo );
            vecpChanFader[i]->SetFaderIsMute ( bStoredFaderIsMute );
            // vecpChanFader[i]->SetGroupID ( iGroupID ); // Must be the last to be set in the fader!
        }
    }
}

void CAudioMixerBoardNu::SetRemoteFaderIsMute ( const int iChannelIdx, const bool bIsMute )
{
    // only apply remote mute state if channel index is valid and the fader is visible
    if ( ( iChannelIdx >= 0 ) && ( iChannelIdx < MAX_NUM_CHANNELS ) )
    {
        if ( vecpChanFader[static_cast<size_t> ( iChannelIdx )]->IsVisible() )
        {
            vecpChanFader[static_cast<size_t> ( iChannelIdx )]->SetRemoteFaderIsMute ( bIsMute );
        }
    }
}

void CAudioMixerBoardNu::UpdateSoloStates()
{
    // first check if any channel has a solo state active
    bool bAnyChannelIsSolo = false;

    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        // check if fader is in use and has solo state active
        if ( vecpChanFader[i]->IsVisible() && vecpChanFader[i]->IsSolo() )
        {
            bAnyChannelIsSolo = true;
            continue;
        }
    }

    // now update the solo state of all active faders
    for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
    {
        if ( vecpChanFader[i]->IsVisible() )
        {
            vecpChanFader[i]->UpdateSoloState ( bAnyChannelIsSolo );
        }
    }
}

void CAudioMixerBoardNu::UpdateGainValue ( const int    iChannelIdx,
                                         const float  fValue,
                                         const bool   bIsMyOwnFader,
                                         const bool   bIsGroupUpdate,
                                         const bool   bSuppressServerUpdate,
                                         const double dLevelRatio )
{
    size_t stChannelIdx = static_cast<size_t> ( iChannelIdx );
    // update current gain
    if ( !bSuppressServerUpdate )
    {
        emit ChangeChanGain ( iChannelIdx, fValue, bIsMyOwnFader );
    }

    // if this fader is selected, all other in the group must be updated as
    // well (note that we do not have to update if this is already a group update
    // to avoid an infinite loop)
    if ( ( vecpChanFader[stChannelIdx]->GetGroupID() != INVALID_INDEX ) && !bIsGroupUpdate )
    {
        for ( size_t i = 0; i < MAX_NUM_CHANNELS; i++ )
        {
            // update rest of faders selected
            if ( vecpChanFader[i]->IsVisible() && ( vecpChanFader[i]->GetGroupID() == vecpChanFader[stChannelIdx]->GetGroupID() ) &&
                 ( i != stChannelIdx ) && ( dLevelRatio >= 0 ) )
            {
                // synchronize faders with moving fader level (it is important
                // to set the group flag to avoid infinite looping)
                vecpChanFader[i]->SetFaderLevel ( vecpChanFader[i]->GetPreviousFaderLevel() * dLevelRatio, true );
            }
        }
    }
}

void CAudioMixerBoardNu::UpdatePanValue ( const int iChannelIdx, const float fValue ) { emit ChangeChanPan ( iChannelIdx, fValue ); }

void CAudioMixerBoardNu::StoreFaderSettings ( CChannelFaderNu* pChanFader )
{
    // if the fader was visible and the name is not empty, we store the old gain
    if ( pChanFader->IsVisible() && !pChanFader->GetReceivedName().isEmpty() )
    {
        CVector<int> viOldStoredFaderLevels ( pSettings->vecStoredFaderLevels );
        CVector<int> viOldStoredPanValues ( pSettings->vecStoredPanValues );
        CVector<int> vbOldStoredFaderIsSolo ( pSettings->vecStoredFaderIsSolo );
        CVector<int> vbOldStoredFaderIsMute ( pSettings->vecStoredFaderIsMute );
        CVector<int> vbOldStoredFaderGroupID ( pSettings->vecStoredFaderGroupID );

        // put new value on the top of the list
        const int iOldIdx = pSettings->vecStoredFaderTags.StringFiFoWithCompare ( pChanFader->GetReceivedName() );

        // current fader level and solo state is at the top of the list
        pSettings->vecStoredFaderLevels[0]  = pChanFader->GetFaderLevel();
        pSettings->vecStoredPanValues[0]    = pChanFader->GetPanValue();
        pSettings->vecStoredFaderIsSolo[0]  = pChanFader->IsSolo();
        pSettings->vecStoredFaderIsMute[0]  = pChanFader->IsMute();
        pSettings->vecStoredFaderGroupID[0] = pChanFader->GetGroupID();
        size_t iTempListCnt                 = 1; // current fader is on top, other faders index start at 1

        for ( size_t iIdx = 0; iIdx < MAX_NUM_STORED_FADER_SETTINGS; iIdx++ )
        {
            // first check if we still have space in our data storage
            if ( iTempListCnt < MAX_NUM_STORED_FADER_SETTINGS )
            {
                // check for the old index of the current entry (this has to be
                // skipped), note that per definition: the old index is an illegal
                // index in case the entry was not present in the vector before
                if ( static_cast<int> ( iIdx ) != iOldIdx )
                {
                    pSettings->vecStoredFaderLevels[iTempListCnt]  = viOldStoredFaderLevels[iIdx];
                    pSettings->vecStoredPanValues[iTempListCnt]    = viOldStoredPanValues[iIdx];
                    pSettings->vecStoredFaderIsSolo[iTempListCnt]  = vbOldStoredFaderIsSolo[iIdx];
                    pSettings->vecStoredFaderIsMute[iTempListCnt]  = vbOldStoredFaderIsMute[iIdx];
                    pSettings->vecStoredFaderGroupID[iTempListCnt] = vbOldStoredFaderGroupID[iIdx];

                    iTempListCnt++;
                }
            }
        }
    }
}

bool CAudioMixerBoardNu::GetStoredFaderSettings ( const QString& strName,
                                                int&           iStoredFaderLevel,
                                                int&           iStoredPanValue,
                                                bool&          bStoredFaderIsSolo,
                                                bool&          bStoredFaderIsMute,
                                                int&           iGroupID )
{
    // only do the check if the name string is not empty
    if ( !strName.isEmpty() )
    {
        for ( size_t iIdx = 0; iIdx < MAX_NUM_STORED_FADER_SETTINGS; iIdx++ )
        {
            // check if fader text is already known in the list
            if ( !pSettings->vecStoredFaderTags[iIdx].compare ( strName ) )
            {
                // copy stored settings values
                iStoredFaderLevel  = pSettings->vecStoredFaderLevels[iIdx];
                iStoredPanValue    = pSettings->vecStoredPanValues[iIdx];
                bStoredFaderIsSolo = pSettings->vecStoredFaderIsSolo[iIdx] != 0;
                bStoredFaderIsMute = pSettings->vecStoredFaderIsMute[iIdx] != 0;
                iGroupID           = pSettings->vecStoredFaderGroupID[iIdx];

                // values found and copied, return OK
                return true;
            }
        }
    }

    // return "not OK" since we did not find matching fader settings
    return false;
}

void CAudioMixerBoardNu::SetChannelLevels ( const CVector<uint16_t>& vecChannelLevel )
{
    const size_t iNumChannelLevels = vecChannelLevel.size();
    size_t       i                 = 0;

    for ( size_t iChId = 0; iChId < MAX_NUM_CHANNELS; iChId++ )
    {
        if ( vecpChanFader[iChId]->IsVisible() && ( i < iNumChannelLevels ) )
        {
            // compute exponential moving average
            vecAvgLevels[iChId] = ( 1.0f - AUTO_FADER_ADJUST_ALPHA ) * vecAvgLevels[iChId] + AUTO_FADER_ADJUST_ALPHA * vecChannelLevel[i];

            vecpChanFader[iChId]->SetChannelLevel ( vecChannelLevel[i++] );

            // show level only if we successfully received levels from the
            // server (if server does not support levels, do not show levels)
            if ( !vecpChanFader[iChId]->GetDisplayChannelLevel() )
            {
                vecpChanFader[iChId]->SetDisplayChannelLevel ( true );
            }
        }
    }
}

void CAudioMixerBoardNu::MuteMyChannel() { SetFaderIsMute ( iMyChannelID, true ); }
