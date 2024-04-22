import QtQuick 2.9
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
    id: idFrontButton
    property string iconName: ""

    height: width
//    icon.name: idFrontButton.iconName
    color: "#97d5e0" // jaune luuni : "#f0bc61"
//    border.color: "#F3C565"
//    border.width: idFrontButton.activeFocus ? 1 : 2
    radius: width * 0.5

    Image {
        anchors.centerIn: parent
        width: parent.width * 3 / 5
        fillMode: Image.PreserveAspectFit
        source: idFrontButton.iconName
    }
}
