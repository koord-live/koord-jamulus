import QtQuick 2.0
import QtWebView 1.1
import QtWebEngine 1.10

Item {

    WebEngineView {
        id: webView
        anchors.fill: parent
        url: _clientdlg.video_url
        onFeaturePermissionRequested: {
            grantFeaturePermission(securityOrigin, feature, true);
        }
    }

}
