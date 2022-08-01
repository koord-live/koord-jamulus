import QtQuick 2.15
import QtWebView 1.1
import QtQuick.Controls 2.15


Rectangle {

    id: grayrect
    color: "gray"
//    anchors.fill: parent

    Text {
            anchors.centerIn: parent
            text: "NO SESSION"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: "Helvetica"
            font.pointSize: 24
            color: "orange"
            visible: webView.loading === false && _clientdlg.video_url === ""
    }

    BusyIndicator{
            anchors.centerIn: parent
            running: webView.loading === true
    }

    WebView {
        id: webView
        anchors.fill: parent
        url: _clientdlg.video_url
        visible: _clientdlg.video_url !== ""
    }

}
