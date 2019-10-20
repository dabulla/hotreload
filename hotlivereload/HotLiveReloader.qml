import QtQuick 2.10
import QtWebSockets 1.0

Item {
    enum ReloadMode {
        Static,             //< Load Qml Element inline (fallback)
        Offline,            //< Use last seen version (cached, local)
        CheckForUpdates,    //< Check for new versions and cache
        LiveReload          //< Connect to url and reaload everytime a websocket signal is received (developer mode)
    }

    id: root
    // url to versions file with host
    property string versionsUrl: "http://127.0.0.1:" + httpPort
    property string httpPortFallback: "8080"
    property string httpPort: "8080"
    // just to store live reload channel
    property string rootFile: "app.qml"
    property alias delay: reloadDoNotFreezeTimer.interval
    readonly property alias status: liveReload.status
    property string host: "127.0.0.1"
    // folder will only be set for specific versions without liveReload
    property string folder: ""
    property int operationMode: HotLiveReloader.ReloadMode.LiveReload
    property bool enableReload: true
    property Component fallback
    property int port: 8081
    property string protocol: "http://"
    readonly property var channels: priv.channels
    readonly property var channelByName: priv.channelsByName
    property string currentChannel
    onCurrentChannelChanged: switchToChannel(currentChannel)
    Item {
        id: priv
        property var channels: ([])
        property var channelsByName: ({})
    }
    function switchToLiveReload() {
        operationMode = HotLiveReloader.ReloadMode.LiveReload
    }

    function switchToChannel(name) {
        let version = priv.channelsByName[name];
        if(typeof version === "undefined") {
            console.log("Unknown Channel could not be loaded: " + name);
            return;
        }
        operationMode = HotLiveReloader.ReloadMode.CheckForUpdates
        protocol = version.protocol
        host     = version.host
        folder   = version.folder
        rootFile = version.rootFile
        httpPortFallback = ((typeof version.port !== "undefined")? version.port : "")
        currentChannel = name;
    }

    Timer {
        // sometimes the app freezed during reload.
        // This timer prevents the issue and leaves some time for
        // qml-engine to clean up
        id: reloadDoNotFreezeTimer
        interval: 100 // 10ms on desktop
        repeat: false
        onTriggered: {
            liveReloadRoot.active = true;
        }
        function syncLiveReload(reloadUrl) {
            liveReloadRoot.active = false;
            if(root.operationMode !== HotLiveReloader.ReloadMode.Static)
                liveReloadRoot.source = reloadUrl
            else
                liveReloadRoot.sourceComponent = root.fallback
            start();
        }
    }
    WebSocket {
        // When the Websocket Server receives a message,
        // the URL changes and triggers a reload via bindings
        // from Loader "liveReloadRoot" to rootUrl
        id: liveReload
        property bool enableLiveReload: root.operationMode === HotLiveReloader.ReloadMode.LiveReload
        active: root.operationMode === HotLiveReloader.ReloadMode.CheckForUpdates || enableLiveReload
        url: "ws://"+ root.host + ":" + root.port
        property string versionPath//: "v0"
        property string rootUrl: "cached" + root.protocol + root.host + (enableLiveReload ? versionPath : (root.httpPortFallback === "" ? "" : (":" + root.httpPortFallback))) + "/" + (enableLiveReload ? "" : root.folder) + "/"
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
    Loader {
        id: versionsLoader
        property string versionsFile: root.versionsUrl + "/Versions.qml"
        source: versionsFile
        onLoaded: {
            if(versionsLoader.status !== Loader.Ready) return;
            console.log("loaded Versions");
            var channels = ([]);
            var channelsByName = ({});
            for( var p in item.publicChannels) {
                let channel = item.publicChannels[p];
                channels.push(channel);
                channelsByName[channel.name] = channel.version;
            }
            priv.channels = channels;
            priv.channelsByName = channelsByName;
        }
    }

    Loader {
        id: liveReloadRoot
        anchors.fill: parent
        asynchronous: false
        property string rootFile: liveReload.rootUrl + root.rootFile
        onRootFileChanged: reloadDoNotFreezeTimer.syncLiveReload(rootFile);
        Item { id: empty }

        // this container will persist a reload. Every change is tracked here.
        property var liveLoadPersistContainer: ({})
        function createConnection(propertyName, loadedItemAppState) {
            var sig = item.appState["on" + propertyName[0].toUpperCase() + propertyName.substring(1) + "Changed"];
            if(typeof sig !== "undefined") {
                sig.connect(function(value) {
                    liveLoadPersistContainer[propertyName] = item.appState[propertyName]
                });
            }
        }
        // initialy and after a reload, values are transfered back from the container
        // and connections are recreated
        onLoaded: {
            // do not reload undefined properties, functions or direct properties of "Item"
            if(typeof item.appState === "undefined") {
                console.log("Application has no property appState for state aware reloading.");
                return;
            }
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
