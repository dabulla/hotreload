import QtQuick 2.0
import Qt.labs.settings 1.0
import QtQuick.LocalStorage 2.12

Item {
    // here should be no bindings because these will be overwritten
    // everything with bindings/computed should stay somewhere else (e.g. appStyle)
    id: appStatePersist

    property string applicationName: "TestApp"
    property int timesPressedTotal: 0
    property string login: "initial name"

    Item {
        //TODO: needed?
        property string applicationName
        id: nonSerializedPropNames
    }

    Settings {
        id: settings
    }
    Item {
        property var db: LocalStorage.openDatabaseSync(appStatePersist.applicationName, "1.0", "The Settings of the App", 10000)
        function createConnection(propertyName) {
            var sig = appStatePersist["on" + propertyName[0].toUpperCase() + propertyName.substring(1) + "Changed"];
            if(typeof sig !== "undefined") {
                var stings = settings;
                sig.connect(function(value) {
                    db.transaction( function(tx) {
                        var result = tx.executeSql('UPDATE Settings SET key=?, value=? where key = ?', [propertyName, JSON.stringify(appStatePersist[propertyName]), propertyName])
                        if( result.rowsAffected === 0) {
                            tx.executeSql('INSERT INTO Settings VALUES(?, ?)', [ propertyName, JSON.stringify(appStatePersist[propertyName]) ]);
                        }
                    });
                });
            }
        }

        Component.onCompleted: {
            db.transaction(
                function(tx) {
                    // Create the database if it doesn't already exist
                    tx.executeSql('CREATE TABLE IF NOT EXISTS Settings(key TEXT, value TEXT)');

                    // Show all added greetings
                    var rs = tx.executeSql('SELECT * FROM Settings');

                    for (var i = 0; i < rs.rows.length; i++) {
                        var key = rs.rows.item(i).key;
                        var value = rs.rows.item(i).value;
                        if( key.startsWith("_")) continue;
                        if( typeof nonSerializedPropNames[key] !== "undefined" ) continue;
                        if( typeof appStatePersist[key] === "function" ) continue;
                        if( typeof appStatePersist[key] === "undefined" ) continue;
                        appStatePersist[key] = JSON.parse(value);
                    }
                    for( var p in appStatePersist) {
                        if( p.startsWith("_")) continue;
                        if( typeof nonSerializedPropNames[p] !== "undefined" ) continue;
                        if( typeof appStatePersist[p] === "function" ) continue;
                        createConnection(p);
                    }
                }
            )
        }
    }
}
