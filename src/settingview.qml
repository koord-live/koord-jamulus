import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {


    Item {
        id: profileLayout
        // username
        // <widget class="QLineEdit" name="pedtAlias">
        // mixer rows
        // <widget class="QSpinBox" name="spnMixerRows">
        SpinBox {
            id: spnMixerRows
            value: _settings.spnMixerRows
        }

    }

    Item {
        id: soundcardLayout

        // Audio Device
         // <widget class="QComboBox" name="cbxSoundcard">

        // Channel Mappings
        // <widget class="QComboBox" name="cbxLInChan"/>
        // <widget class="QComboBox" name="cbxRInChan"/>
        // //
        // <widget class="QComboBox" name="cbxLOutChan"/>
        // <widget class="QComboBox" name="cbxROutChan"/>

        // Channel Setup box
         // <widget class="QComboBox" name="cbxAudioChannels">
        ComboBox {
            id:     cbxAudioChannels
            model: ListModel {
                id: chmodel
                ListElement { text: "Mono" }
                ListElement { text: "Mono-in/Stereo-out" }
                ListElement { text: "Stereo" }
            }
            // value: _settings.cbxAudioChannels
            onCurrentIndexChanged: _settings.cbxAudioChannels = currentIndex

        }

        // Quality box
        // <widget class="QComboBox" name="cbxAudioQuality">
        ComboBox {
            id: cbxAudioQuality
            model: ListModel {
                id: aqmodel
                ListElement { text: "Low" }
                ListElement { text: "Normal" }
                ListElement { text: "High" }
            }
            onCurrentIndexChanged: _settings.cbxAudioQuality = currentIndex
        }

        // New client level
        // <widget class="QDial" name="newInputLevelDial">
        Dial {
            id: newInputLevelDial
            from: 0
            to: 100
            value: _settings.edtNewClientLevel
            onMoved: _settings.edtNewClientLevel = value
            // onValueChanged: _settings.edtNewClientLevel = value
        }

        // TO PUT IN MAIN SECTION?
            // from: 0 # AUD_FADER_IN_MIN
            // to: 100 # AUD_FADER_IN_MAX
        Dial {
            id: panDial
            from: 0
            to: 100
            value: _settings.panDialLevel
            onMoved: _settings.panDialLevel = value
            // onValueChanged: _settings.panDialLevel = value
        }

        // Feedback box
        // <widget class="QCheckBox" name="chbDetectFeedback">
        CheckBox {
            id: chbDetectFeedback
            checked: _settings.chbDetectFeedback
            // pressed:
        }

        // Soundcard Buffer Delay
        // <widget class="QRadioButton" name="rbtBufferDelayPreferred">
        // <widget class="QRadioButton" name="rbtBufferDelayDefault">
        // <widget class="QRadioButton" name="rbtBufferDelaySafe">
        ColumnLayout {
            RadioButton {
                id: rbtBufferDelayPreferred
                // text: _settings.GetSndCrdBufferDelayString(FRAME_SIZE_FACTOR_PREFERRED * SYSTEM_FRAME_SIZE_SAMPLES)
                checked: _settings.rbtBufferDelayPreferred
            }
            RadioButton {
                id: rbtBufferDelayDefault
                // text: _settings.GetSndCrdBufferDelayString(FRAME_SIZE_FACTOR_DEFAULT * SYSTEM_FRAME_SIZE_SAMPLES)
                checked: _settings.rbtBufferDelayDefault
            }
            RadioButton {
                id: rbtBufferDelaySafe
                // text: _settings.GetSndCrdBufferDelayString(FRAME_SIZE_FACTOR_SAFE * SYSTEM_FRAME_SIZE_SAMPLES)
                checked: _settings.rbtBufferDelaySafe
            }
        }


    }


    Item {
        id: networklayout

        // Audio Stream rate
        // <widget class="QLabel" name="lblUpstreamValue">

        // Jitter Buffer
        // <widget class="QCheckBox" name="chbAutoJitBuf">
        // <widget class="QSlider" name="sldNetBuf">
        Slider {
            id:     sldNetBuf
            from:   MIN_NET_BUF_SIZE_NUM_BL
            to:     MAX_NET_BUF_SIZE_NUM_BL
            value:  _settings.sldNetBuf
            moved:  _settings.sldNetBuf = value
        }

        // <widget class="QSlider" name="sldNetBufServer">
        Slider {
            id:     sldNetBufServer
            from:   MIN_NET_BUF_SIZE_NUM_BL
            to:     MAX_NET_BUF_SIZE_NUM_BL
            value:  _settings.sldNetBufServer
            moved:  _settings.sldNetBufServer = value
        }

        // Small Network Buffers
        // <widget class="QCheckBox" name="chbEnableOPUS64">
        CheckBox {
            id: chbEnableOPUS64
            checked: _settings.chbEnableOPUS64
        }


    }


}
