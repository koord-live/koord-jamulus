import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import "."

Item {

    id: mainview
    anchors.fill: parent

    LevelMeter {
        id: levelMeterRectangle
        anchors.centerIn: parent
        levelValue: 0.8 // _main.level // Link to C++ property
    }
}
