import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 40
    height: 300
    color: "white"
    // radius: 3
    border.color: "black"
    border.width: 1

    property alias levelValue: levelBar.heightPercentage

    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            id: levelContainer
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.preferredHeight: root.height
            width: 35
            height: root.height
            color: "black"
            border.color: "orange"
            border.width: 5
            // radius: 5

            Rectangle {
                id: levelBar
                width: parent.width * 0.8
                height: levelContainer.height * heightPercentage
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter // Center horizontally
                // Layout.alignment: Qt.AlignBottom
                color: "green"
                border.color: "yellow"
                border.width: 3
                radius: 3

                property real heightPercentage: 0 // Level in percentage, can be updated from C++
            }
        }

        // Text {
        //     text: "Level: " + levelBar.heightPercentage + "%"
        //     anchors.horizontalCenter: parent.horizontalCenter
        // }
    }
}
