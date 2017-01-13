import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Item {
    width: 640
    height: 480
    property alias bubble: bubble
    property alias mainWindow: mainWindow

    Rectangle {
        id: mainWindow
        color: "#ffffff"
        anchors.fill: parent

        Bubble {
            id: bubble
        }
    }

}
