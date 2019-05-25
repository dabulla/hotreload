import QtQuick 2.10
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtWebSockets 1.0

// This is a boilerplate-qml that displays connection information at the top
// to connect to a hot reload server. "TestApplication.qml" is reloaded whenever the server
// tells the template about a change.
// This boilerplate is not realoded and will persist. Thus it can be used to restore
// application state after a reload.
// The property "appState" of "TestApplication" is tracked and carried over this way.

ApplicationWindow {
    id: window
    visible: true

    width: 480
    height: width*16.0/9.0
    title: qsTr("Live Reload Test App")

    Timer {
        id: reloadDoNotFreezeTimer
        interval: 10
        repeat: false
        onTriggered: {
            liveReloadRoot.active = true;
        }
        function syncLiveReload(reloadUrl) {
            liveReloadRoot.active = false;
            liveReloadRoot.source = reloadUrl
            start();
        }
    }
    WebSocket {
        id: liveReload
        active: true
        url: "ws://"+hostTextfield.text + ":" + portTextfield.text
        property string versionPath//: "v0"
        property string rootUrl: "http://" + hostTextfield.text + versionPath
        onTextMessageReceived: {
            console.log("Reloading from url: " + message)
            versionPath = message
        }
        onStatusChanged: {
            if (status === WebSocket.Error) {
                console.log("Live Reload error: " + errorString);
            } else if (status === WebSocket.Closed) {
                console.log("Live Reload socket closed.");
            } else if (status === WebSocket.Open) {
            }
        }
    }
    RowLayout {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        visible: liveReload.status !== WebSocket.Open //&& liveReload.status !== WebSocket.Connecting
        Label { text: "Livereload Url:" }
        TextField {
            id: hostTextfield
            text: "127.0.0.1"
        }
        TextField {
            id: portTextfield
            text: "8081"
            width: 50
        }
        Button {
            text: "X"
        }
    }
    Loader {
        id: liveReloadRoot
        anchors.fill: parent
        asynchronous: false
        property string rootFile: liveReload.rootUrl + "TestApplication.qml"
        //source:
        onRootFileChanged: reloadDoNotFreezeTimer.syncLiveReload(rootFile);
        Item { id: empty }

        property var liveLoadPersistContainer: ({})
        function createConnection(propertyName, loadedItemAppState) {
            var sig = item.appState["on" + propertyName[0].toUpperCase() + propertyName.substring(1) + "Changed"];
            if(typeof sig !== "undefined") {
                sig.connect(function(value) {
                    liveLoadPersistContainer[propertyName] = item.appState[propertyName]
                });
            }
        }
        onLoaded: {
            if(typeof item.appState === "undefined") return;
            var loadedItemAppState = item.appState
            for( var p in loadedItemAppState) {
                if( typeof loadedItemAppState[p] === "function" ) continue;
                if( typeof loadedItemAppState[p] === "undefined" ) continue;
                if( typeof empty[p] !== "undefined" ) continue;
                if( typeof liveLoadPersistContainer[p] !== "undefined" ) {
                    loadedItemAppState[p] = liveLoadPersistContainer[p];
                }
                createConnection(p, loadedItemAppState);
            }
        }
    }
}
