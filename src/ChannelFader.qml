import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: channelFader
    anchors.fill: parent
    radius: 5
    border.color: "black"
    border.width: 1

    ColumnLayout {
        Layout.fillHeight: true
        anchors.fill: parent

        RowLayout {
            Layout.leftMargin: 3
            Layout.topMargin: 5
            Label {
                id: panLabel
                text: "PAN"
            }
        }

        // Pan Knob
        Dial {
            id: panKnob
            from: -1
            to: 1
            value: 0
            stepSize: 0.1
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            contentItem: Text {
                text: "PAN"
                color: "white"
                font.pixelSize: 12
            }
        }

        RowLayout {
            spacing: 5
            Layout.alignment: Qt.AlignHCenter

            // Level Meter
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 300
                Layout.preferredWidth: 30

                LevelMeter {
                    id: levelMeterRectangleUser
                    anchors.fill: parent
                    Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                    levelValueL: 0.8 // _main.level // Link to C++ property
                    levelValueR: 0.4 // _main.level // Link to C++ property
                }
            }

            // Fader
            Slider {
                id: volumeFader
                orientation: Qt.Vertical
                from: 0
                to: 100
                value: 50
                stepSize: 1
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 300
                width: 30
            }
        }

        // GRPMTSOLO box
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 5

            RowLayout {
                spacing: 5
                Layout.alignment: Qt.AlignHCenter

                Button {
                    text: "M"
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    font.bold: true
                    icon.color: "orange"
                }
                Button {
                    text: "S"
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    font.bold: true
                    // background: "yellow"
                }
            }

            Button {
                text: "GRP"
                Layout.preferredWidth: 60
                Layout.preferredHeight: 30
                font.bold: true
            }

        }

        // Username label
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter

                border.color: "black"
                border.width: 1
                radius: 5

                Label {
                    text: "username#1"
                    anchors.centerIn: parent // do this because parent is not layout
                }
            }
        }

    }
}

