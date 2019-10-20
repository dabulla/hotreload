import QtQuick 2.0
import QtQuick.Controls 2.5
//import "hotlivereload"
//import Qt3D.Core 2.12
//import Qt3D.Render 2.12
//import QtQuick.Scene3D 2.12


Item {
    id: root

    property var appState: appState
    AppState {
        id: appState
    }

    property string text: "Hello from Beat Channel!\nButton has been pressed " + appState.timesPressed + " times! (total " + appStatePersist.timesPressedTotal + ")"
    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: appState.currentPage
        onCurrentIndexChanged: appState.currentPage = currentIndex
        Column {
            spacing: 20
            Text {
                id: textLabel
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.text
                font.pixelSize: 20
            }

            Button {
                id: button
                anchors.horizontalCenter: parent.horizontalCenter
                text: "+1"
                onClicked: {
                    appState.timesPressed++
                    appStatePersist.timesPressedTotal++
                }
            }
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                Label {
                    anchors.verticalCenter: textField.verticalCenter
                    text: "Betaname: "
                }
                TextField {
                    id: textField
                    width: 230
                    text: appStatePersist.login
                    onTextChanged: appStatePersist.login = text
                }
            }
            Label {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                text: appStatePersist.login
            }
    //        Scene3D {
    //            id:scene3d
    //            width: 200//parent.width
    //            height: 200
    //            focus: true
    //            aspects: ["input", "logic"]
    //            cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

    //            AnimatedEntity {}
    //        }
        }
        Item {
            Column {
                anchors.fill: parent
                anchors.leftMargin: 20
                spacing: 50
                Item { height: 5; width: 1 }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Available Versions"
                    font.pixelSize: 30
                }

                Repeater {
                    model: liveReloader.channels
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: versionContentColumn.height
                        Column {
                            id: versionContentColumn
                            Text { text: modelData.name }
                            Text { text: modelData.version.versionName }
                            Button {
                                text: "Download"
                                onClicked: {
                                    liveReloader.switchToChannel(modelData.name);
                                }
                            }
                        }
                    }
                }
                Item { height: 5; width: 1 }
                Button {
                    text: "Live Reload"
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: {
                        liveReloader.switchToLiveReload()
                    }
                }
            }
        }
    }
    PageIndicator {
        id: indicator

        count: swipeView.count
        currentIndex: swipeView.currentIndex

        anchors.bottom: swipeView.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
