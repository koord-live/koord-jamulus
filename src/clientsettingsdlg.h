
#pragma once

#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QWhatsThis>
#include <QTimer>
#include <QSlider>
#include <QRadioButton>
#include <QMenuBar>
#include <QLayout>
#include <QButtonGroup>
#include <QMessageBox>
#include "global.h"
#include "util.h"
#include "client.h"
#include "settings.h"
#include "multicolorled.h"
#include "ui_clientsettingsdlgbase.h"

/* Definitions ****************************************************************/
// update time for GUI controls
#define DISPLAY_UPDATE_TIME 1000 // ms

/* Classes ********************************************************************/
class CClientSettingsDlg : public CBaseDlg, private Ui_CClientSettingsDlgBase
{
    Q_OBJECT

public:
    CClientSettingsDlg ( CClient* pNCliP, CClientSettings* pNSetP, QWidget* parent = nullptr );

    void UpdateUploadRate();
    void UpdateDisplay();
    void UpdateSoundDeviceChannelSelectionFrame();

    void SetEnableFeedbackDetection ( bool enable );

protected:
    void    UpdateJitterBufferFrame();
    void    UpdateSoundCardFrame();
    void    UpdateDirectoryComboBox();
    void    UpdateAudioFaderSlider();
    QString GenSndCrdBufferDelayString ( const int iFrameSize, const QString strAddText = "" );

    virtual void showEvent ( QShowEvent* );

    CClient*         pClient;
    CClientSettings* pSettings;
    QTimer           TimerStatus;
    QButtonGroup     SndCrdBufferDelayButtonGroup;

public slots:
    void OnTimerStatus() { UpdateDisplay(); }
    void OnNetBufValueChanged ( int value );
    void OnNetBufServerValueChanged ( int value );
    void OnAutoJitBufStateChanged ( int value );
    void OnEnableOPUS64StateChanged ( int value );
    void OnFeedbackDetectionChanged ( int value );
    void OnCustomDirectoriesEditingFinished();
    void OnNewClientLevelEditingFinished() { pSettings->iNewClientFaderLevel = edtNewClientLevel->text().toInt(); }
    void OnNewClientLevelChanged();
    void OnInputBoostChanged();
    void OnSndCrdBufferDelayButtonGroupClicked ( QAbstractButton* button );
    void OnSoundcardActivated ( int iSndDevIdx );
    void OnLInChanActivated ( int iChanIdx );
    void OnRInChanActivated ( int iChanIdx );
    void OnLOutChanActivated ( int iChanIdx );
    void OnROutChanActivated ( int iChanIdx );
    void OnAudioChannelsActivated ( int iChanIdx );
    void OnAudioQualityActivated ( int iQualityIdx );
    void OnGUIDesignActivated ( int iDesignIdx );
    void OnMeterStyleActivated ( int iMeterStyleIdx );
    void OnLanguageChanged ( QString strLanguage ) { pSettings->strLanguage = strLanguage; }
    void OnAliasTextChanged ( const QString& strNewName );
//    void OnInstrumentActivated ( int iCntryListItem );
//    void OnCountryActivated ( int iCntryListItem );
//    void OnCityTextChanged ( const QString& strNewName );
//    void OnSkillActivated ( int iCntryListItem );
//    void OnTabChanged();
//    void OnMakeTabChange ( int iTabIdx );
    void OnAudioPanValueChanged ( int value );
#if defined( _WIN32 ) && !defined( WITH_JACK )
    // Only include this slot for Windows when JACK is NOT used
    void OnDriverSetupClicked();
    void OnSoundcardReactivate();
#endif


signals:
    void GUIDesignChanged();
    void MeterStyleChanged();
    void AudioChannelsChanged();
    void CustomDirectoriesChanged();
    void NumMixerPanelRowsChanged ( int value );

};