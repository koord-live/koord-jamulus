import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {


    Item {
        id: profileLayout
        // username
        // <widget class="QLineEdit" name="pedtAlias">
        TextField {
            id: inputField
            width: 200
            placeholderText: "Enter alias here"
            text: "Default text"

            onTextChanged: {
                console.log("Text changed to: " + text)
            }
        }


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
        ComboBox {
            id:                         cbxSoundcard
            model:                      _settings.slSndCrdDevNames
            currentIndex:               _settings.slSndCrdDevIndex
            onCurrentIndexChanged:      _settings.slSndCrdDevIndex = currentIndex
        }

        Text {
            text:                       "Selected Name: " + _settings.slSndCrdDevNames[_settings.slSndCrdDevIndex]
            anchors.top:                cbxSoundcard.bottom
            anchors.topMargin:          20
            anchors.horizontalCenter:   cbxSoundcard.horizontalCenter
        }

        // Channel Mappings
        // <widget class="QComboBox" name="cbxLInChan"/>
        ComboBox {
            id:                         cbxLInChan
            model:                      _settings.sndCardNumInputChannels
            currentIndex:               _settings.sndCardLInChannel
            onCurrentIndexChanged:      _settings.sndCardLInChannel = currentIndex
        }


        // <widget class="QComboBox" name="cbxRInChan"/>
        ComboBox {
            id:                         cbxRInChan
            model:                      _settings.sndCardNumInputChannels
            currentIndex:               _settings.sndCardRInChannel
            onCurrentIndexChanged:      _settings.sndCardRInChannel = currentIndex
        }


        // <widget class="QComboBox" name="cbxLOutChan"/>
        ComboBox {
            id:                         cbxLOutChan
            model:                      _settings.sndCardNumInputChannels
            currentIndex:               _settings.sndCardLOutChannel
            onCurrentIndexChanged:      _settings.sndCardLOutChannel = currentIndex
        }

        // <widget class="QComboBox" name="cbxROutChan"/>
        ComboBox {
            id:                         cbxROutChan
            model:                      _settings.sndCardNumInputChannels
            currentIndex:               _settings.sndCardROutChannel
            onCurrentIndexChanged:      _settings.sndCardROutChannel = currentIndex
        }

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
                text: _settings.getSndCrdBufferDelayString(FRAME_SIZE_FACTOR_PREFERRED * SYSTEM_FRAME_SIZE_SAMPLES)
                checked: _settings.rbtBufferDelayPreferred
            }
            RadioButton {
                id: rbtBufferDelayDefault
                text: _settings.getSndCrdBufferDelayString(FRAME_SIZE_FACTOR_DEFAULT * SYSTEM_FRAME_SIZE_SAMPLES)
                checked: _settings.rbtBufferDelayDefault
            }
            RadioButton {
                id: rbtBufferDelaySafe
                text: _settings.getSndCrdBufferDelayString(FRAME_SIZE_FACTOR_SAFE * SYSTEM_FRAME_SIZE_SAMPLES)
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
        CheckBox {
            id:     chbAutoJitBuf
            checked: true
            text:   "Auto"
        }

        // <widget class="QSlider" name="sldNetBuf">
        Slider {
            id:     sldNetBuf
            from:   MIN_NET_BUF_SIZE_NUM_BL
            to:     MAX_NET_BUF_SIZE_NUM_BL
            value:  _settings.sldNetBuf
            moved:  _settings.sldNetBuf = value
            enabled: !chbAutoJitBuf
        }

        // <widget class="QSlider" name="sldNetBufServer">
        Slider {
            id:     sldNetBufServer
            from:   MIN_NET_BUF_SIZE_NUM_BL
            to:     MAX_NET_BUF_SIZE_NUM_BL
            value:  _settings.sldNetBufServer
            moved:  _settings.sldNetBufServer = value
            enabled: !chbAutoJitBuf
        }

        // Small Network Buffers
        // <widget class="QCheckBox" name="chbEnableOPUS64">
        CheckBox {
            id: chbEnableOPUS64
            checked: _settings.chbEnableOPUS64
        }


    }


}
