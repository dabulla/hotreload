import QtQuick 2.0
import QtQuick.Controls 2.5

Item {
    id: root
    property var appState: appState
    AppState {
        id: appState
    }
    AppStatePersistent {
        id: appStatePersist
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
//        Label {
//            width: parent.width
//            horizontalAlignment: Qt.AlignHCenter
//            text: appStatePersist.login
//        }
    }
}
