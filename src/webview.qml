import QtQuick 2.0
import QtWebView 1.1

Item {

//    Text {
//        id: text
//        text: qsTr("Hello world")

//        anchors.centerIn: parent
//    }

    WebView {
        id: webView
        anchors.fill: parent
        url: "https://koord.live"
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
