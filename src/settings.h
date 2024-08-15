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

#pragma once

#include <QDomDocument>
#include <QFile>
#include <QSettings>
#include <QDir>
#ifndef HEADLESS
#    include <QApplication>
#    include <QMessageBox>
#endif
#include "global.h"
#ifndef SERVER_ONLY
#    include "client.h"
#endif
#include "server.h"
#include "util.h"
#include <QQuickView>
#include "QQmlContext"


/* Classes ********************************************************************/
class CSettings : public QObject
{
    Q_OBJECT

public:
    CSettings() :
        vecWindowPosMain(), // empty array
        strLanguage ( "" ),
        strFileName ( "" )
    {
        QObject::connect ( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &CSettings::OnAboutToQuit );
#ifndef HEADLESS

        // The Jamulus App will be created as either a QCoreApplication or QApplication (a subclass of QGuiApplication).
        // State signals are only delivered to QGuiApplications, so we determine here whether we instantiated the GUI.
        const QGuiApplication* pGApp = dynamic_cast<const QGuiApplication*> ( QCoreApplication::instance() );

        if ( pGApp != nullptr )
        {
#    ifndef QT_NO_SESSIONMANAGER
            QObject::connect (
                pGApp,
                &QGuiApplication::saveStateRequest,
                this,
                [=] ( QSessionManager& ) { Save ( false ); },
                Qt::DirectConnection );

#    endif
            QObject::connect ( pGApp, &QGuiApplication::applicationStateChanged, this, [=] ( Qt::ApplicationState state ) {
                if ( Qt::ApplicationActive != state )
                {
                    Save ( false );
                }
            } );
        }
#endif
    }

    void Load ( const QList<QString>& CommandLineOptions );
    void Save ( bool isAboutToQuit );

    // common settings
    QByteArray vecWindowPosMain;
    QString    strLanguage;

protected:
    virtual void WriteSettingsToXML ( QDomDocument& IniXMLDocument, bool isAboutToQuit )                              = 0;
    virtual void ReadSettingsFromXML ( const QDomDocument& IniXMLDocument, const QList<QString>& CommandLineOptions ) = 0;

    void ReadFromFile ( const QString& strCurFileName, QDomDocument& XMLDocument );
    void WriteToFile ( const QString& strCurFileName, const QDomDocument& XMLDocument );
    void SetFileName ( const QString& sNFiName, const QString& sDefaultFileName );

    // The following functions implement the conversion from the general string
    // to base64 (which should be used for binary data in XML files). This
    // enables arbitrary utf8 characters to be used as the names in the GUI.
    //
    // ATTENTION: The "FromBase64[...]" functions must be used with caution!
    //            The reason is that if the FromBase64ToByteArray() is used to
    //            assign the stored value to a QString, this is incorrect but
    //            will not generate a compile error since there is a default
    //            conversion available for QByteArray to QString.
    QString    ToBase64 ( const QByteArray strIn ) const { return QString::fromLatin1 ( strIn.toBase64() ); }
    QString    ToBase64 ( const QString strIn ) const { return ToBase64 ( strIn.toUtf8() ); }
    QByteArray FromBase64ToByteArray ( const QString strIn ) const { return QByteArray::fromBase64 ( strIn.toLatin1() ); }
    QString    FromBase64ToString ( const QString strIn ) const { return QString::fromUtf8 ( FromBase64ToByteArray ( strIn ) ); }

    // init file access function for read/write
    void SetNumericIniSet ( QDomDocument& xmlFile, const QString& strSection, const QString& strKey, const int iValue = 0 );

    bool GetNumericIniSet ( const QDomDocument& xmlFile,
                            const QString&      strSection,
                            const QString&      strKey,
                            const int           iRangeStart,
                            const int           iRangeStop,
                            int&                iValue );

    void SetFlagIniSet ( QDomDocument& xmlFile, const QString& strSection, const QString& strKey, const bool bValue = false );

    bool GetFlagIniSet ( const QDomDocument& xmlFile, const QString& strSection, const QString& strKey, bool& bValue );

    // actual working function for init-file access
    QString GetIniSetting ( const QDomDocument& xmlFile, const QString& sSection, const QString& sKey, const QString& sDefaultVal = "" );

    void PutIniSetting ( QDomDocument& xmlFile, const QString& sSection, const QString& sKey, const QString& sValue = "" );

    QString strFileName;

public slots:
    void OnAboutToQuit() { Save ( true ); }
};

// #######################################################################################################################################
// CLIENT STUFF _ THE ONLY SETTINGS THAT MATTER !!!

#ifndef SERVER_ONLY
class CClientSettings : public CSettings
{
    Q_OBJECT

    Q_PROPERTY(int edtNewClientLevel READ edtNewClientLevel WRITE setEdtNewClientLevel NOTIFY newClientLevelChanged)
    Q_PROPERTY(int panDialLevel READ panDialLevel WRITE setPanDialLevel NOTIFY panDialLevelChanged)
    Q_PROPERTY(int uploadRate READ uploadRate WRITE setUploadRate NOTIFY uploadRateChanged)
    Q_PROPERTY(int sldNetBuf READ sldNetBuf WRITE setSldNetBuf NOTIFY sldNetBufChanged)
    Q_PROPERTY(int sldNetBufServer READ sldNetBufServer WRITE setSldNetBufServer NOTIFY sldNetBufServerChanged)
    Q_PROPERTY(int cbxAudioChannels READ cbxAudioChannels WRITE setCbxAudioChannels NOTIFY cbxAudioChannelsChanged)
    Q_PROPERTY(int cbxAudioQuality READ cbxAudioQuality WRITE setCbxAudioQuality NOTIFY cbxAudioQualityChanged)
    Q_PROPERTY(int dialInputBoost READ dialInputBoost WRITE setDialInputBoost NOTIFY dialInputBoostChanged)
    Q_PROPERTY(int spnMixerRows READ spnMixerRows WRITE setSpnMixerRows NOTIFY spnMixerRowsChanged)
    Q_PROPERTY(bool chbDetectFeedback READ chbDetectFeedback WRITE setChbDetectFeedback NOTIFY chbDetectFeedbackChanged)
    Q_PROPERTY(bool chbEnableOPUS64 READ chbEnableOPUS64 WRITE setChbEnableOPUS64 NOTIFY chbEnableOPUS64Changed)
    Q_PROPERTY(bool rbtBufferDelayPreferred READ rbtBufferDelayPreferred WRITE SetRbtBufferDelayPreferred NOTIFY rbtBufferDelayPreferredChanged)
    Q_PROPERTY(bool rbtBufferDelayDefault READ rbtBufferDelayDefault WRITE SetRbtBufferDelayDefault NOTIFY rbtBufferDelayDefaultChanged)
    Q_PROPERTY(bool rbtBufferDelaySafe READ rbtBufferDelaySafe WRITE SetRbtBufferDelaySafe NOTIFY rbtBufferDelaySafeChanged)


public:
    CClientSettings ( CClient* pNCliP, const QString& sNFiName ) :
        CSettings(),
        vecStoredFaderTags ( MAX_NUM_STORED_FADER_SETTINGS, "" ),
        vecStoredFaderLevels ( MAX_NUM_STORED_FADER_SETTINGS, AUD_MIX_FADER_MAX ),
        vecStoredPanValues ( MAX_NUM_STORED_FADER_SETTINGS, AUD_MIX_PAN_MAX / 2 ),
        vecStoredFaderIsSolo ( MAX_NUM_STORED_FADER_SETTINGS, false ),
        vecStoredFaderIsMute ( MAX_NUM_STORED_FADER_SETTINGS, false ),
        vecStoredFaderGroupID ( MAX_NUM_STORED_FADER_SETTINGS, INVALID_INDEX ),
        vstrIPAddress ( MAX_NUM_SERVER_ADDR_ITEMS, "" ),
        iNewClientFaderLevel ( 100 ),
        iInputBoost ( 1 ),
        iSettingsTab ( SETTING_TAB_AUDIONET ),
        bConnectDlgShowAllMusicians ( true ),
        eChannelSortType ( ST_NO_SORT ),
        iNumMixerPanelRows ( 1 ),
        vstrDirectoryAddress ( MAX_NUM_SERVER_ADDR_ITEMS, "" ),
        eDirectoryType ( AT_DEFAULT ),
        bEnableFeedbackDetection ( true ),
        bEnableAudioAlerts ( false ),
        vecWindowPosSettings(), // empty array
        vecWindowPosChat(),     // empty array
        vecWindowPosConnect(),  // empty array
        bWindowWasShownSettings ( false ),
        bWindowWasShownChat ( false ),
        bWindowWasShownConnect ( false ),
        bOwnFaderFirst ( false ),
        pClient ( pNCliP )
    {
        SetFileName ( sNFiName, DEFAULT_INI_FILE_NAME );

        // transitional settings view
        settingsView = new QQuickView();
        // QWidget *settingsContainer = QWidget::createWindowContainer(settingsView, this);
        settingsView->setSource(QUrl("qrc:/settingview.qml"));
        // settingsTab->layout()->addWidget(settingsContainer);
        QQmlContext* settingsContext = settingsView->rootContext();
        settingsContext->setContextProperty("_settings", this);

        // Do I put signal-slot connections here ???? even necessary?
        // QObject::connect ( edtNewClientLevel(), &QLineEdit::editingFinished, this, &CClientSettings::changeNewClientLevel );

        // UpdateAudioFaderSlider();

        UpdateJitterBufferFrame();

        // UpdateSoundDeviceChannelSelectionFrame();


        setCbxAudioChannels(pClient->GetAudioChannels());


        setSpnMixerRows(iNumMixerPanelRows);

        setChbDetectFeedback(bEnableFeedbackDetection);

    }


    // TODO: Convert / BRING IN FROM clientsettingsdlg.h
    QQuickView*     settingsView;

    int edtNewClientLevel() const;
    void setEdtNewClientLevel(const int newClientLevel );

    int panDialLevel() const;
    void setPanDialLevel( const int dialLevel );

    int sldNetBuf() const;
    void setSldNetBuf( const int setBufVal );

    int sldNetBufServer() const;
    void setSldNetBufServer( const int setServerBufVal );

    int cbxAudioChannels() const;
    void setCbxAudioChannels( const int chanIdx );

    int cbxAudioQuality() const;
    void setCbxAudioQuality( const int qualityIdx );

    int dialInputBoost() const;
    void setDialInputBoost( const int inputBoost );

    int spnMixerRows() const;
    void setSpnMixerRows( const int mixerRows );

    bool chbDetectFeedback();
    void setChbDetectFeedback( bool detectFeeback );

    bool chbEnableOPUS64();
    void setChbEnableOPUS64( bool enableOPUS64 );

    bool rbtBufferDelayPreferred();
    void setRbtBufferDelayPreferred( bool enableBufDelPref );

    bool rbtBufferDelayDefault();
    void setRbtBufferDelayDefault( bool enableBufDelDef );

    bool rbtBufferDelaySafe();
    void setRbtBufferDelaySafe( bool enableBufDelSafe );


    void UpdateUploadRate(); // maintained for now
    int uploadRate() const;
    void setUploadRate();

    // void UpdateDisplay(); // don't need this?

    // DO THIS LATER, A BIT MORE INVOLVED
    // void UpdateSoundDeviceChannelSelectionFrame();

    // void SetEnableFeedbackDetection ( bool enable );
    // endof TODO


    QString GenSndCrdBufferDelayString ( const int iFrameSize, const QString strAddText = "" );


    void LoadFaderSettings ( const QString& strCurFileName );
    void SaveFaderSettings ( const QString& strCurFileName );

    // general settings
    CVector<QString> vecStoredFaderTags;
    CVector<int>     vecStoredFaderLevels;
    CVector<int>     vecStoredPanValues;
    CVector<int>     vecStoredFaderIsSolo;
    CVector<int>     vecStoredFaderIsMute;
    CVector<int>     vecStoredFaderGroupID;
    CVector<QString> vstrIPAddress;
    int              iNewClientFaderLevel;
    int              iInputBoost;
    int              iSettingsTab;
    bool             bConnectDlgShowAllMusicians;
    EChSortType      eChannelSortType;
    int              iNumMixerPanelRows;
    CVector<QString> vstrDirectoryAddress;
    EDirectoryType   eDirectoryType;
    int              iCustomDirectoryIndex; // index of selected custom directory
    bool             bEnableFeedbackDetection;
    bool             bEnableAudioAlerts;

    // window position/state settings
    QByteArray vecWindowPosSettings;
    QByteArray vecWindowPosChat;
    QByteArray vecWindowPosConnect;
    bool       bWindowWasShownSettings;
    bool       bWindowWasShownChat;
    bool       bWindowWasShownConnect;
    bool       bOwnFaderFirst;

    // for Test mode setting
    QByteArray strTestMode;

    // for QML display
    // int         m_uploadRate;

protected:
    virtual void WriteSettingsToXML ( QDomDocument& IniXMLDocument, bool isAboutToQuit ) override;
    virtual void ReadSettingsFromXML ( const QDomDocument& IniXMLDocument, const QList<QString>& ) override;

    void ReadFaderSettingsFromXML ( const QDomDocument& IniXMLDocument );
    void WriteFaderSettingsToXML ( QDomDocument& IniXMLDocument );

    CClient* pClient;

    // TODO: convert / pull in from clientsettingdlg.h
    void    UpdateJitterBufferFrame();
    // void    UpdateSoundCardFrame();
    // void    UpdateDirectoryComboBox();
    // void    UpdateAudioFaderSlider();
    // QString GenSndCrdBufferDelayString ( const int iFrameSize, const QString strAddText = "" );

    // virtual void showEvent ( QShowEvent* );

    // CClient*         pClient;
    // CClientSettings* pSettings;
    // QTimer           TimerStatus;
    // QButtonGroup     SndCrdBufferDelayButtonGroup;
    // // endof TODO ------------------


// This is where we hook up controls in QML
public slots:
    // TO CONVERT TO QML-WORLD
//     void OnTimerStatus() { UpdateDisplay(); }
//     void OnNetBufValueChanged ( int value );
//     void OnNetBufServerValueChanged ( int value );
//     void OnAutoJitBufStateChanged ( int value );
//     void OnEnableOPUS64StateChanged ( int value );
//     void OnFeedbackDetectionChanged ( int value );
//     void OnCustomDirectoriesEditingFinished();
    // void onNewClientLevelEditingFinished() { iNewClientFaderLevel = edtNewClientLevel(); }
    // void changeNewClientLevel() { setEdtNewClientLevel( iNewClientFaderLevel ); }
    // void changePanDialLevel() { setPanDialLevel( panDialLevel ); }
//     void OnNewClientLevelChanged();
//     void OnInputBoostChanged();
//     void OnSndCrdBufferDelayButtonGroupClicked ( QAbstractButton* button );
//     void OnSoundcardActivated ( int iSndDevIdx );
//     void OnLInChanActivated ( int iChanIdx );
//     void OnRInChanActivated ( int iChanIdx );
//     void OnLOutChanActivated ( int iChanIdx );
//     void OnROutChanActivated ( int iChanIdx );
//     void OnAudioChannelsActivated ( int iChanIdx );
//     void OnAudioQualityActivated ( int iQualityIdx );
//     void OnGUIDesignActivated ( int iDesignIdx );
//     void OnMeterStyleActivated ( int iMeterStyleIdx );
//     void OnLanguageChanged ( QString strLanguage ) { pSettings->strLanguage = strLanguage; }
//     void OnAliasTextChanged ( const QString& strNewName );
//     //    void OnInstrumentActivated ( int iCntryListItem );
//     //    void OnCountryActivated ( int iCntryListItem );
//     //    void OnCityTextChanged ( const QString& strNewName );
//     //    void OnSkillActivated ( int iCntryListItem );
//     //    void OnTabChanged();
//     //    void OnMakeTabChange ( int iTabIdx );
//     void OnAudioPanValueChanged ( int value );
// #if defined( _WIN32 ) && !defined( WITH_JACK )
//     // Only include this slot for Windows when JACK is NOT used
//     void OnDriverSetupClicked();
//     void OnSoundcardReactivate();
// #endif


signals:
    void newClientLevelChanged();
    void sldNetBufChanged();
    void sldNetBufServerChanged();
    void panDialLevelChanged();
    void uploadRateChanged();
    void cbxAudioChannelsChanged();
    void cbxAudioQualityChanged();
    void dialInputBoostChanged();
    void spnMixerRowsChanged();
    void chbDetectFeedbackChanged();
    void chbEnableOPUS64Changed();
    void rbtBufferDelayPreferredChanged();
    void rbtBufferDelayDefaultChanged();
    void rbtBufferDelaySafeChanged();

    // TODO: convert / pull in from clientsettingdlg.h
    // void GUIDesignChanged();
    // void MeterStyleChanged();
    // void AudioChannelsChanged();
    // void CustomDirectoriesChanged();
    // void NumMixerPanelRowsChanged ( int value );
    // endof TODO ---------

private:


};
#endif





// class CServerSettings : public CSettings
// {
// public:
//     CServerSettings ( CServer* pNSerP, const QString& sNFiName ) : CSettings(), pServer ( pNSerP )
//     {
//         SetFileName ( sNFiName, DEFAULT_INI_FILE_NAME_SERVER );
//     }

// protected:
//     virtual void WriteSettingsToXML ( QDomDocument& IniXMLDocument, bool isAboutToQuit ) override;
//     virtual void ReadSettingsFromXML ( const QDomDocument& IniXMLDocument, const QList<QString>& CommandLineOptions ) override;

//     CServer* pServer;
// };
