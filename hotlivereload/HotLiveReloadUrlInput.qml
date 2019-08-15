import QtQuick 2.10
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.4
import QtWebSockets 1.0

Item {
    // This is a minmal UI to enter a URL of HotReloadServer
    id: hotReloadLayer
    property HotLiveReloader liveReloader

    anchors.fill: parent
    visible: liveReloader.enableLiveReload
             && liveReloader.status !== WebSocket.Open
             //&& liveReload.status !== WebSocket.Connecting
    z: 100

    Rectangle {
    AppStatePersistent {
        id: hotReloadAppState
        applicationName: "HotReloadUi"
        property string hotReloadHost: "127.0.0.1"
        property string hotReloadPort: "8081"
    }
    Binding {
        target: liveReloader
        property: "host"
        value: hotReloadAppState.hotReloadHost
    }
    Binding {
        target: liveReloader
        property: "port"
        value: hotReloadAppState.hotReloadPort
    }

        anchors.fill: hotReloadInput
        z: 99
        color: "white"
    }

    RowLayout {
        id: hotReloadInput
        z: 100
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        Label { text: "Livereload Url:" }
        TextField {
            id: hostTextfield
            text: hotReloadAppState.hotReloadHost
            onTextChanged: hotReloadAppState.hotReloadHost = text
        }
        TextField {
            id: portTextfield
            text: hotReloadAppState.hotReloadPort
            onTextChanged: hotReloadAppState.hotReloadPort = text
            width: 50
        }
        Button {
            text: "X"
            onClicked: hotReloadLayer.visible = false
        }
    }
}
