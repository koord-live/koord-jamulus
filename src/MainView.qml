import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."


Rectangle {
    visible: true
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"
        radius: 10
        border.color: "#d9d9d9"

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // Left Control Panel
            Rectangle {
                width: 200
                color: "#ffffff"
                radius: 10
                border.color: "#d9d9d9"
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10 // Adds margins around the content
                    spacing: 10

                    Text {
                        text: "INPUT"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }

                    // Level Meters (Simulated with Rectangles for illustration)
                    // ColumnLayout {
                    //     spacing: 4
                    //     Layout.alignment: Qt.AlignHCenter
                    //     Repeater {
                    //         model: 10
                    //         Rectangle {
                    //             width: 20
                    //             height: 15
                    //             color: "#000000"
                    //             radius: 5
                    //         }
                    //     }
                    // }
                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: 300
                        Layout.preferredWidth: 40

                        LevelMeter {
                            id: levelMeterRectangle
                            anchors.fill: parent
                            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                            levelValueL: 0.8 // _main.level // Link to C++ property
                            levelValueR: 0.9 // _main.level // Link to C++ property
                        }
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter

                        ColumnLayout {
                            Layout.alignment: Qt.AlignHCenter

                            Button {
                                text: "M"
                                Layout.preferredWidth: 50
                                Layout.preferredHeight: 50
                                Layout.alignment: Qt.AlignHCenter
                                font.bold: true
                            }

                            Text {
                                text: "mute"
                                font.pixelSize: 10
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }

                        // Knobs for Pan and Boost
                        ColumnLayout {
                            Layout.alignment: Qt.AlignHCenter

                            Dial {
                                Layout.preferredWidth: 50
                                Layout.preferredHeight: 50
                                // color: "#f0a500"
                                // radius: 20
                            }
                            Text {
                                text: "pan"
                                font.pixelSize: 10
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }

                    // Network Check Section
                    GridLayout {
                        Layout.alignment: Qt.AlignCenter
                        columns: 2

                        Text {
                            text: "PING"
                        }
                        Rectangle {
                            width: 40
                            height: 15
                            color: "#000000"
                            Label {
                                anchors.centerIn: parent
                                text: "23"
                                color: "green"
                                font.bold: true
                            }
                        }

                        Text {
                            text: "DELAY"
                        }
                        Rectangle {
                            width: 40
                            height: 15
                            color: "#000000"
                            Label {
                                anchors.centerIn: parent
                                color: "red"
                                text: "56"
                                font.bold: true
                            }
                        }

                        Text {
                            text: "JITTER"
                        }
                        Rectangle {
                            width: 40
                            height: 15
                            color: "#000000"
                            Label {
                                anchors.centerIn: parent
                                text: "--"
                                color: "green"
                                font.bold: true
                            }
                        }
                    }
                }
            }

            // Main Area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#f5f5f5"
                border.color: "#d9d9d9"
                radius: 10

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 20

                    // Connect / control area
                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter

                        Button {
                            text: "Connect..."
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 50
                            font.bold: true
                        }
                    }

                    // mixerboard area
                    Rectangle {
                        Layout.preferredHeight: 550
                        Layout.preferredWidth: 90

                        ChannelFader {
                            id: channelFader1
                            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                            // levelValue: 0.8 // _main.level // Link to C++ property
                        }
                    }

                }
            }
        }
    }
}
