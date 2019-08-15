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

    property string text: "Button has been pressed " + appState.timesPressed + " times! (total " + appStatePersist.timesPressedTotal + ")"
    Column {
        spacing: 20
        anchors.centerIn: parent
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
                text: "Name: "
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
}
