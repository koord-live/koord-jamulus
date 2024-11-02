import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: levelContainer
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "black"
    radius: 2

    property alias levelValueL: levelBarL.heightPercentage
    property alias levelValueR: levelBarR.heightPercentage

    RowLayout {
        id: levelBarsLayout
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: levelBarL
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.preferredWidth: levelContainer.width * 0.35
            Layout.preferredHeight: levelContainer.height * heightPercentage
            radius: 1
            gradient: Gradient {
                GradientStop { position: 1.0; color: "green" }
                GradientStop { position: 0.5; color: "yellow" }
                GradientStop { position: 0.0; color: "red" }
            }
            property real heightPercentage: 0.5 // Default 50% height for demonstration
        }

        Rectangle {
            id: levelBarR
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.preferredWidth: levelContainer.width * 0.35
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
