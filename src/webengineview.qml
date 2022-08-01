import QtQuick 2.15
import QtWebView 1.1
import QtWebEngine 1.10
import QtQuick.Controls 2.15

Rectangle {

    id: orangerect
    color: "gray"
    anchors.fill: parent

    Text {
            anchors.fill: parent
            text: "NO SESSION"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: "Helvetica"
            font.pointSize: 24
            color: "orange"
            visible: webView.loading === false
    }


    BusyIndicator{
            anchors.centerIn: parent
            running: webView.loading === true
    }

    WebEngineView {
        id: webView
        anchors.fill: parent
        url: _clientdlg.video_url
        onFeaturePermissionRequested: {
            grantFeaturePermission(securityOrigin, feature, true);
        }
        visible: _clientdlg.video_url !== ""
    }

}
