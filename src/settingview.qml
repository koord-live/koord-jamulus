import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    visible: true
    width: 1280
    height: 960

    ScrollView {
        width: parent.width
        height: parent.height

        Column {
            width: parent.width

            Rectangle {
                id: profileLayout
                width: parent.width
                color: "#f0f0f0"
                border.color: "#888"
                border.width: 2
                radius: 10
                height: childrenRect.height + 20
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    width: parent.width * 0.9
                    spacing: 10
                    padding: 10
                // username

                    Text {
                        text: "Username"
                    }

                    TextField {
                        id: inputField
                        width: 200
                        placeholderText: "Enter alias here"
                        text: "Enter username here"
                        onTextChanged:       _settings.pedtAlias = text;
                    }


                // mixer row count
                // <widget class="QSpinBox" name="spnMixerRows">
                    Text {
                        text: "Mixer Rows count"
                    }

                    SpinBox {
                        id: spnMixerRows
                        value: _settings.spnMixerRows
                        onValueChanged: _settings.spnMixerRows = value;
                    }
                }
            }

            Rectangle {
                id: soundsettingsLayout
                width: parent.width
                color: "#e0f7fa"
                border.color: "#00796b"
                border.width: 2
                radius: 10
                height: childrenRect.height + 20
                anchors.horizontalCenter: parent.horizontalCenter

                // Audio Device
                 // <widget class="QComboBox" name="cbxSoundcard">
                Column {
                    width: parent.width * 0.9
                    spacing: 10
                    padding: 10
                    Text {
                        text: "Audio Device"
                    }

                    ComboBox {
                        id:                         cbxSoundcard
                        model:                      _settings.slSndCrdDevNames
                        displayText:                _settings.slSndCardDev //FIXME - update in client.h|cpp - pSettings pointer migrate from clientdlg. reason: this doesn't update if device selection fails
                        onCurrentTextChanged:       _settings.slSndCrdDev = currentText;
                    }

                    Text {
                        text:                       "Selected Name: " + _settings.slSndCrdDev
                        // anchors.top:                cbxSoundcard.bottom
                        // anchors.topMargin:          20
                        // anchors.horizontalCenter:   cbxSoundcard.horizontalCenter
                    }

                // Channel Mappings

                    Text {
                        text: "Input channel - LEFT"
                    }
                    ComboBox {
                        id:                         cbxLInChan
                        model:                      _settings.sndCrdInputChannelNames
                        displayText:                _settings.sndCardLInChannel
                        onCurrentTextChanged:       _settings.sndCardLInChannel = currentText
                    }

                    Text {
                        text: "Input channel - RIGHT"
                    }
                    ComboBox {
                        id:                         cbxRInChan
                        model:                      _settings.sndCrdInputChannelNames
                        displayText:                _settings.sndCardRInChannel
                        onCurrentTextChanged:       _settings.sndCardRInChannel = currentText
                    }

                    Text {
                        text: "Output channel - LEFT"
                    }
                    ComboBox {
                        id:                         cbxLOutChan
                        model:                      _settings.sndCrdOutputChannelNames
                        displayText:                _settings.sndCardLOutChannel
                        onCurrentTextChanged:       _settings.sndCardLOutChannel = currentText
                    }

                    Text {
                        text: "Output channel - RIGHT"
                    }
                    ComboBox {
                        id:                         cbxROutChan
                        model:                      _settings.sndCrdOutputChannelNames
                        displayText:                _settings.sndCardROutChannel
                        onCurrentTextChanged:       _settings.sndCardRoutChannel = currentText
                    }

                // Channel Setup box
                 // <widget class="QComboBox" name="cbxAudioChannels">
                    Text {
                        text: "Mono/Stereo mode"
                    }
                    ComboBox {
                        id:     cbxAudioChannels
                        model: ListModel {
                            id: chmodel
                            ListElement { text: "Mono" }
                            ListElement { text: "Mono-in/Stereo-out" }
                            ListElement { text: "Stereo" }
                        }
                        displayText: textAt(_settings.cbxAudioChannels)
                        onCurrentIndexChanged: _settings.cbxAudioChannels = currentIndex

                    }

                // Quality box
                // <widget class="QComboBox" name="cbxAudioQuality">
                    Text {
                        text: "Audio quality"
                    }
                    ComboBox {
                        id: cbxAudioQuality
                        model: ListModel {
                            id: aqmodel
                            ListElement { text: "Low" }
                            ListElement { text: "Normal" }
                            ListElement { text: "High" }
                        }
                        displayText: textAt(_settings.cbxAudioQuality) // this ensures correct value shows at startup
                        onActivated: _settings.cbxAudioQuality = currentIndex
                    }

                // New client level
                // <widget class="QDial" name="newInputLevelDial">
                    Text {
                        text: "New client level: " + _settings.edtNewClientLevel
                    }
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
                    Text {
                        text: "Pan: " + _settings.panDialLevel
                    }
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
                    Text {
                        text: "Feedback Detection"
                    }
                    CheckBox {
                        id: chbDetectFeedback
                        checked: _settings.chbDetectFeedback
                        // pressed:
                    }

                // Soundcard Buffer Delay
                // <widget class="QRadioButton" name="rbtBufferDelayPreferred">
                // <widget class="QRadioButton" name="rbtBufferDelayDefault">
                // <widget class="QRadioButton" name="rbtBufferDelaySafe">
                    Text {
                        text: "Buffer Size"
                    }
                    ColumnLayout {
                        RadioButton {
                            id: rbtBufferDelayPreferred
                            text:  _settings.sndCrdBufferDelayPreferred
                            checked: _settings.rbtBufferDelayPreferred
                            onClicked: _settings.rbtBufferDelayPreferred = true
                        }
                        RadioButton {
                            id: rbtBufferDelayDefault
                            text:  _settings.sndCrdBufferDelayDefault
                            checked: _settings.rbtBufferDelayDefault
                            onClicked: _settings.rbtBufferDelayDefault = true
                        }
                        RadioButton {
                            id: rbtBufferDelaySafe
                            text:  _settings.sndCrdBufferDelaySafe
                            checked: _settings.rbtBufferDelaySafe
                            onClicked: _settings.rbtBufferDelaySafe = true
                        }
                    }
                }
            }

            Rectangle {
                id: networksettingsLayout
                width: parent.width
                color: "#ffebee"
                border.color: "#d32f2f"
                border.width: 2
                radius: 10
                height: childrenRect.height + 20
                anchors.horizontalCenter: parent.horizontalCenter

                // Audio Stream rate
                // <widget class="QLabel" name="lblUpstreamValue">

                // Jitter Buffer
                // <widget class="QCheckBox" name="chbAutoJitBuf">
                Column {
                    width: parent.width * 0.9
                    spacing: 10
                    padding: 10

                    Text {
                        text: "Jitter Buffer - Auto"
                    }
                    CheckBox {
                        id:     chbAutoJitBuf
                        checked: _settings.chbAutoJitBuf
                        onCheckedChanged: _settings.chbAutoJitBuf = checked
                        text:   "Auto"
                    }

                // <widget class="QSlider" name="sldNetBuf">
                    Text {
                        text: "Client buffer"
                    }
                    Slider {
                        id:     sldNetBuf
                        from:   _settings.sldNetBufMin // 20 // MIN_NET_BUF_SIZE_NUM_BL
                        to:     _settings.sldNetBufMax // 100 //MAX_NET_BUF_SIZE_NUM_BL
                        value:  _settings.sldNetBuf
                        onMoved:  _settings.sldNetBuf = value
                        enabled: !_settings.chbAutoJitBuf
                    }

                    Text {
                        text: "Server Buffer"
                    }
                    // <widget class="QSlider" name="sldNetBufServer">
                    Slider {
                        id:     sldNetBufServer
                        from:   20 // MIN_NET_BUF_SIZE_NUM_BL
                        to:     100 // MAX_NET_BUF_SIZE_NUM_BL
                        value:  _settings.sldNetBufServer
                        onMoved:  _settings.sldNetBufServer = value
                        enabled: _settings.chbAutoJitBuf === false
                    }


                // Small Network Buffers
                // <widget class="QCheckBox" name="chbEnableOPUS64">
                    Text {
                        text: "Small network buffers"
                    }
                    CheckBox {
                        id: chbEnableOPUS64
                        checked: _settings.chbEnableOPUS64
                        onCheckStateChanged: _settings.chbEnableOPUS64 = checkState
                    }
                }
            }
        }
    }

}
