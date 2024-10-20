import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {

    ColumnLayout {

        ColumnLayout {
            Layout.fillWidth: true
            // color: "blue"
            id: profileLayout
            // username
            // <widget class="QLineEdit" name="pedtAlias">
            RowLayout {
                Text {
                    text: "Username"
                }

                TextField {
                    id: inputField
                    width: 200
                    placeholderText: "Enter alias here"
                    text: "Default text"

                    onTextChanged: {
                        console.log("Text changed to: " + text)
                    }
                }
            }

            // mixer rows
            // <widget class="QSpinBox" name="spnMixerRows">
            RowLayout {
                Text {
                    text: "Mixer Rows count"
                }

                SpinBox {
                    id: spnMixerRows
                    value: _settings.spnMixerRows
                }
            }

        }

        ColumnLayout {
            Layout.fillWidth: true
            id: soundcardLayout
            // color: "yellow"

            // Audio Device
             // <widget class="QComboBox" name="cbxSoundcard">
            RowLayout {
                Text {
                    text: "Audio Device"
                }

                ComboBox {
                    id:                         cbxSoundcard
                    model:                      _settings.slSndCrdDevNames
                    displayText:                "" //FIXME - this doesn't update if device selection fails
                    onCurrentTextChanged:       _settings.slSndCrdDev = currentText;
                }
            }

            Text {
                text:                       "Selected Name: " + _settings.slSndCrdDev
                // anchors.top:                cbxSoundcard.bottom
                // anchors.topMargin:          20
                // anchors.horizontalCenter:   cbxSoundcard.horizontalCenter
            }

            // Channel Mappings
            // <widget class="QComboBox" name="cbxLInChan"/>
            RowLayout {
                Text {
                    text: "Input channel - Left"
                }
                ComboBox {
                    id:                         cbxLInChan
                    model:                      _settings.sndCardNumInputChannels
                    currentIndex:               _settings.sndCardLInChannel
                    // displayText:                _settings.sndCardLInChannel + 1
                    onCurrentIndexChanged:      _settings.sndCardLInChannel = currentIndex
                }
            }

            // <widget class="QComboBox" name="cbxRInChan"/>
            RowLayout {
                Text {
                    text: "Input channel - Right"
                }
                ComboBox {
                    id:                         cbxRInChan
                    model:                      _settings.sndCardNumInputChannels
                    currentIndex:               _settings.sndCardRInChannel
                    // displayText:                _settings.sndCardRInChannel + 1
                    onCurrentIndexChanged:      _settings.sndCardRInChannel = currentIndex
                }
            }


            // <widget class="QComboBox" name="cbxLOutChan"/>
            RowLayout {
                Text {
                    text: "Output channel - LEFT"
                }
                ComboBox {
                    id:                         cbxLOutChan
                    model:                      _settings.sndCardNumOutputChannels
                    currentIndex:               _settings.sndCardLOutChannel
                    onCurrentIndexChanged:      _settings.sndCardLOutChannel = currentIndex
                }
            }

            // <widget class="QComboBox" name="cbxROutChan"/>
            RowLayout {
                Text {
                    text: "Output channel - RIGHT"
                }
                ComboBox {
                    id:                         cbxROutChan
                    model:                      _settings.sndCardNumOutputChannels
                    currentIndex:               _settings.sndCardROutChannel
                    onCurrentIndexChanged:      _settings.sndCardROutChannel = currentIndex
                }
            }

            // Channel Setup box
             // <widget class="QComboBox" name="cbxAudioChannels">
            RowLayout {
                Text {
                    text: "mono/stereo mode"
                }
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
            }

            // Quality box
            // <widget class="QComboBox" name="cbxAudioQuality">
            RowLayout {
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
                    onCurrentIndexChanged: _settings.cbxAudioQuality = currentIndex
                }
            }

            // New client level
            // <widget class="QDial" name="newInputLevelDial">
            RowLayout {
                Text {
                    text: "New client level"
                }
                Dial {
                    id: newInputLevelDial
                    from: 0
                    to: 100
                    value: _settings.edtNewClientLevel
                    onMoved: _settings.edtNewClientLevel = value
                    // onValueChanged: _settings.edtNewClientLevel = value
                }
            }

            // TO PUT IN MAIN SECTION?
                // from: 0 # AUD_FADER_IN_MIN
                // to: 100 # AUD_FADER_IN_MAX
            RowLayout {
                Text {
                    text: "Pan"
                }
                Dial {
                    id: panDial
                    from: 0
                    to: 100
                    value: _settings.panDialLevel
                    onMoved: _settings.panDialLevel = value
                    // onValueChanged: _settings.panDialLevel = value
                }
            }

            // Feedback box
            // <widget class="QCheckBox" name="chbDetectFeedback">
            RowLayout {
                Text {
                    text: "Feedback Detection"
                }
                CheckBox {
                    id: chbDetectFeedback
                    checked: _settings.chbDetectFeedback
                    // pressed:
                }
            }

            // Soundcard Buffer Delay
            // <widget class="QRadioButton" name="rbtBufferDelayPreferred">
            // <widget class="QRadioButton" name="rbtBufferDelayDefault">
            // <widget class="QRadioButton" name="rbtBufferDelaySafe">
            RowLayout {
                Text {
                    text: "Buffer Size"
                }
                ColumnLayout {
                    RadioButton {
                        id: rbtBufferDelayPreferred
                        text:  "2" //  _settings.getSndCrdBufferDelayString(FRAME_SIZE_FACTOR_PREFERRED * SYSTEM_FRAME_SIZE_SAMPLES)
                        checked: _settings.rbtBufferDelayPreferred
                    }
                    RadioButton {
                        id: rbtBufferDelayDefault
                        text:  "20" // _settings.getSndCrdBufferDelayString(FRAME_SIZE_FACTOR_DEFAULT * SYSTEM_FRAME_SIZE_SAMPLES)
                        checked: _settings.rbtBufferDelayDefault
                    }
                    RadioButton {
                        id: rbtBufferDelaySafe
                        text:  "200" //  _settings.getSndCrdBufferDelayString(FRAME_SIZE_FACTOR_SAFE * SYSTEM_FRAME_SIZE_SAMPLES)
                        checked: _settings.rbtBufferDelaySafe
                    }
                }
            }


        }


        ColumnLayout {
            Layout.fillWidth: true
            id: networklayout
            // color: "orange"

            // Audio Stream rate
            // <widget class="QLabel" name="lblUpstreamValue">

            // Jitter Buffer
            // <widget class="QCheckBox" name="chbAutoJitBuf">
            RowLayout {
                Text {
                    text: "Jitter Buffer - Auto"
                }
                CheckBox {
                    id:     chbAutoJitBuf
                    checked: true
                    text:   "Auto"
                }
            }

            // <widget class="QSlider" name="sldNetBuf">
            RowLayout {
                Text {
                    text: "Client buffer"
                }
                Slider {
                    id:     sldNetBuf
                    from:   20 // MIN_NET_BUF_SIZE_NUM_BL
                    to:     100 //MAX_NET_BUF_SIZE_NUM_BL
                    value:  _settings.sldNetBuf
                    onMoved:  _settings.sldNetBuf = value
                    enabled: !chbAutoJitBuf
                }
            }

            RowLayout {
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
                    enabled: !chbAutoJitBuf
                }
            }

            // Small Network Buffers
            // <widget class="QCheckBox" name="chbEnableOPUS64">
            RowLayout {
                Text {
                    text: "Small network buffers"
                }
                CheckBox {
                    id: chbEnableOPUS64
                    checked: _settings.chbEnableOPUS64
                }
            }


        }

    }

}
