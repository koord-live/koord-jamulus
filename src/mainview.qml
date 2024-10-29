import QtQuick 2.15
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

                }
            }

        }
    }
}
