import QtQuick 2.0

// Here all public versions are listed
// There must be an item with the property "publicChannels"
// Each Channel has a Name and the current version
// Each version has protocol, host, folder, rootFile set which is enough to feed HotLiveReloader
Item {
    property var publicChannels: [tagStable, tagBeta, tagPublic]
    Item {
        id: priv
        property string host: "127.0.0.1"
        property string folder: "tags"
    }

    Item {
        id: tagStable
        property string name: "Stable"
        property var version: ver101
    }
    Item {
        id: tagBeta
        property string name: "Beta"
        property var version: ver231
    }
    Item {
        id: tagPublic
        property string name: "Public Github"
        property var version: verPublic
    }
    Item {
        id: ver100
        property int versionNumber: 100
        property string versionName: "v1.0.0"
        property string protocol: "http://"
        property string host: priv.host
        property string port: "8080"
        property string folder: "v100"
        property string rootFile: "TestApplication.qml"
        property var changeLog: [
            "Initial Version"
        ]
    }
    Item {
        id: ver101
        property int versionNumber: 101
        property string versionName: "v1.0.1"
        property string protocol: "http://"
        property string host: priv.host
        property string port: "8080"
        property string folder: ""
        property string rootFile: "TestApplication.qml"
        property var changeLog: [
            "minorUpdate"
        ]
    }
    Item {
        id: ver231
        property int versionNumber: 231
        property string versionName: "v2.3.1"
        property string protocol: "http://"
        property string host: priv.host
        property string port: "8080"
        property string folder: "betachannel_version"
        property string rootFile: "TestApplication.qml"
        property var changeLog: [
            "major big update"
        ]
    }
    Item {
        id: verPublic
        property int versionNumber: 50
        property string versionName: "public"
        property string protocol: "https://"
        property string host: "raw.githubusercontent.com"
        //property string port: "443"
        property string folder: "dabulla/hotreload/master/testapp/"
        property string rootFile: "TestApplication.qml"
        property var changeLog: [
            "this is from github"
        ]
    }
}
