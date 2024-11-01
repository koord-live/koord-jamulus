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
                            levelValue: 0.8 // _main.level // Link to C++ property
                        }
                    }

                    // Mute Button
                    // Rectangle {
                    //     width: 40
                    //     height: 40
                    //     color: "#c0c0c0"
                    //     radius: 5
                    //     Layout.alignment: Qt.AlignHCenter
                    //     Text {
                    //         text: "M"
                    //         anchors.centerIn: parent
                    //         font.pixelSize: 20
                    //         color: "#ffffff"
                    //     }
                    // }

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
                    ColumnLayout {
                        spacing: 4
                        Layout.alignment: Qt.AlignCenter
                        Text {
                            text: "NET CHECK"
                            font.pixelSize: 12
                        }

                        RowLayout {
                            Text { text: "ping" }
                            Rectangle {
                                width: 40
                                height: 15
                                color: "#000000"
                                Label {
                                    anchors.centerIn: parent
                                    text: "23"
                                    color: "green"
                                }
                            }
                        }
                        RowLayout {
                            Text { text: "delay" }
                            Rectangle {
                                width: 40
                                height: 15
                                color: "#000000"
                                Label {
                                    anchors.centerIn: parent
                                    color: "green"
                                    text: "23"
                                }
                            }
                        }
                        RowLayout {
                            Text { text: "jitter" }
                            Rectangle {
                                width: 40
                                height: 15
                                color: "#000000"
                                Label {
                                    anchors.centerIn: parent
                                    text: "23"
                                    color: "green"
                                }
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

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 20

                    // Custom Button Style Using a Non-native Control
                    Button {
                        text: "Connect..."
                        width: 100
                        height: 40
                        // background: Rectangle {
                        //     color: "#ff8c00"
                        //     radius: 5
                        // }
                        font.bold: true
                    }

                }
            }
        }
    }
}
