import QtQuick 2.0
import QtWebView 1.1

Rectangle {

//    Text {
//        id: text
//        text: qsTr("Hello world")

//        anchors.centerIn: parent
//    }

//    Text {
//        text: _clientdlg.video_url
//        font.family: "Helvetica"
//        font.pointSize: 24
//        color: "red"
//    }
    id: orangerect
    color: "orange"
    anchors.fill: parent

    Text {
            anchors.fill: parent
            text: "NO SESSION"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: "Helvetica"
            font.pointSize: 24
            color: "black"
    }

    WebView {
        id: webView
        anchors.fill: parent
        url: _clientdlg.video_url
//        url: "https://clx5x3k0.kv.koord.live:30803/video"
    //    onLoadingChanged: function(loadRequest) {
    //        if (loadRequest.errorString)
    //            console.error(loadRequest.errorString);
    //    }
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
