import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: parent.width
    height: parent.height
    color: "white"
    border.color: "black"
    border.width: 1
    radius: 3

    property alias levelValue: levelBar.heightPercentage

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            id: levelContainer
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.preferredHeight: root.height
            Layout.preferredWidth: root.width * 0.5
            color: "black"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                Layout.fillWidth: true
                Layout.fillHeight: true

                // Spacer to push levelBar to the bottom
                Item {
                    Layout.fillHeight: true
                }

                Rectangle {
                    id: levelBar
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: levelContainer.width * 0.5
                    Layout.preferredHeight: levelContainer.height * heightPercentage
                    radius: 1
                    gradient: Gradient {
                        GradientStop { position: 1.0; color: "green" }
                        GradientStop { position: 0.5; color: "yellow" }
                        GradientStop { position: 0.0; color: "red" }
                    }
                    property real heightPercentage: 0.5 // Default 50% height for demonstration
                }
            }
        }
    }
}
