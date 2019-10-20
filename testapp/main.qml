import QtQuick 2.10
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.4
import QtWebSockets 1.0
import "../hotlivereload"

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

    AppStatePersistent {
        // here should be no bindings because these will be overwritten
        // everything with bindings/computed should stay somewhere else (e.g. appStyle)
        id: appStatePersist

        applicationName: "TestApp"
        property int timesPressedTotal: 0
        property string login: "initial name"
        property string currentUpdateChannel
        property string currentVersion
    }
    HotLiveReloader {
        id: liveReloader

        anchors.fill: parent
        rootFile: "TestApplication.qml"

        operationMode: HotLiveReloader.ReloadMode.LiveReload
        fallback: Component {
            TestApplication {}
        }
        currentChannel: appStatePersist.currentUpdateChannel
        onCurrentChannelChanged: {
            console.log("Switched to channel " + currentChannel + " from " + appStatePersist.currentUpdateChannel)
            appStatePersist.currentUpdateChannel = currentChannel
            //appStatePersist.currentVersion = channelByName[currentChannel].versionName
        }
    }
    HotLiveReloadUrlInput {
        anchors.fill: parent
        liveReloader: liveReloader
    }
}
