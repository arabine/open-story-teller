import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.3

import "."

Rectangle {
    id: idStoryView
    anchors.fill: parent
    color: '#5586a4' // vert lunni: "#45ADAF"
    Screen.orientationUpdateMask:  Qt.LandscapeOrientation | Qt.PortraitOrientation

    Screen.onPrimaryOrientationChanged: {
        console.log("orinetation changed, width: " + width )
    }

    //property bool isPortrait: Screen.primaryOrientation === Qt.PortraitOrientation || Screen.primaryOrientation === Qt.InvertedPortraitOrientation

    property bool isPortrait: idStoryView.width < idStoryView.height
    property int sectionWidth: idStoryView.isPortrait ? idStoryView.width : idStoryView.width / 2

    property int sectionWidth1: idStoryView.isPortrait ? idStoryView.width : idStoryView.width * 2 / 3
    property int sectionWidth2: idStoryView.isPortrait ? idStoryView.width : idStoryView.width * 1 / 3

    property int sectionHeight: idStoryView.isPortrait ? idStoryView.height / 2 : idStoryView.height

    property int oneTime: 0;

    function setImage(img) {
      //  imageDisplay.source = img;
    }

    Component.onCompleted: {
      //  idLcdArea.initialize();
        storyTeller.initialize();
    }

    Connections
    {
        target: storyTeller
        function onSigShowImage() {
            setImage(storyTeller.getImage());
        }

        function onSigClearScreen() {
            setImage("default_image.bmp");
        }

        function onSigDrawPixel(r, g, b, x, y) {
            idLcdArea.drawPixel(r, g, b, x, y);
        }
    }

    Rectangle {
        x: 0
        y: idStoryView.isPortrait ? idStoryView.sectionHeight : 0
        width: sectionWidth1
        height: sectionHeight
        color: "transparent"


        Canvas {
//            id: mycanvas
            id: idLcdArea
            width: 320
            height: 240
            anchors.centerIn: parent



                onPaint: {
                    if (idStoryView.oneTime === 0)
                    {
                        idStoryView.oneTime = 1;
                        var ctx = getContext("2d");
                        ctx.fillStyle = Qt.rgba(1, 0, 0, 1);
                        ctx.fillRect(0, 0, width, height);
                    }
                }

            function initialize()
            {
                var ctx = getContext("2d");
                ctx.fillStyle = Qt.rgba(1, 0, 0, 1);
                ctx.fillRect(0, 0, width, height);
                requestPaint();
            }

            function drawPixel(r, g, b, x, y)
            {
                var ctx = getContext("2d");
                ctx.fillStyle = Qt.rgba(r/255, g/255, b/255, 1);
                ctx.fillRect( x, y, 1, 1 );
                requestPaint();
            }
        }
/*
        Rectangle {
            id: idLcdArea
            anchors.centerIn: parent
            width: idStoryView.isPortrait ?  height * 4 / 3 : idStoryView.sectionWidth1  * 4 / 5
            height: idStoryView.isPortrait ? idStoryView.sectionHeight  * 4 / 5 :  width * 3 / 4
            color: "#38A0A2"




//            Image {
//                z: 1000
//                id: imageDisplay
//                fillMode: Image.PreserveAspectFit
//                anchors.fill: parent
//                source: "osb-logo-scale320x240.png"
//            }

            MouseArea {
                anchors.fill: parent
                preventStealing: true
                property real velocity: 0.0
                property int xStart: 0
                property int xPrev: 0
                property bool tracing: false
                onPressed: {
                    xStart = mouse.x
                    xPrev = mouse.x
                    velocity = 0
                    tracing = true
                    console.log( " pressed  "+xStart)
                    console.log( " pressed  "+xPrev)
                }
                onPositionChanged: {
                    if ( !tracing ) return
                    var currVel = mouse.x > xPrev ? (mouse.x-xPrev) : xPrev - mouse.x
                    velocity = (velocity + currVel)/2.0
                    xPrev = mouse.x

                    if ( velocity > 15 && mouse.x > parent.width*0.2 ) {
                        tracing = false
                    }

                    if ( velocity > 15 && mouse.x < parent.width*0.2 ) {
                        tracing = false
                    }
                }
                onReleased: {
                    tracing = false
                    if ( velocity > 15 && mouse.x > parent.width*0.2 ) {
                        // SWIPE DETECTED !! EMIT SIGNAL or DO your action
                        console.log(">>>>>>>");

                        storyTeller.next();
                    }

                    if ( velocity > 15 && mouse.x < parent.width*0.2 ) {
                        // SWIPE DETECTED !! EMIT SIGNAL or DO your action
                        console.log("<<<<<<<");

                        storyTeller.previous();
                    }
                }
            }
        } // écran lcd
        */

        Label {
            anchors.top: idLcdArea.bottom
            anchors.topMargin: 10
            width: parent.width
            text: storyTeller.message
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Rectangle {
        id: idFrontButtons
        x: idStoryView.isPortrait ? 0 : idStoryView.sectionWidth1
        y: idStoryView.isPortrait ? idStoryView.sectionHeight * 2 : 0
        width: sectionWidth2
        height: sectionHeight
        color: "transparent"

        Button {
            text: "\u2630";
            anchors.right: parent.right
            anchors.rightMargin: 20
            width: 40
            onClicked:  folderDialog.open()
        }

        FileDialog {
            id: folderDialog
            selectFolder: true
            onAccepted: {
//                console.log("You chose: " + folderDialog.fileUrl)
                storyTeller.saveSettings(folderDialog.fileUrl);
            }
        }

        FrontButton {
            width: idFrontButtons.width  / 4
            x: (idFrontButtons.width - width) / 2
            y: (idFrontButtons.height / 4) - (width / 2)
            iconName: "icons/home.png"
        }

        FrontButton {
            width: idFrontButtons.width  / 4
            x: (idFrontButtons.width - width) / 2
            y: (idFrontButtons.height * 2 / 4) - (width / 2)
            iconName: "icons/pause.png"
        }

        FrontButton {
            width: idFrontButtons.width  / 3
            x: (idFrontButtons.width - width) / 2
            y: (idFrontButtons.height * 3 / 4) - (width / 2)
            iconName: "icons/ok.png"

            MouseArea {
                anchors.fill: parent
                onClicked: storyTeller.okButton()
            }
        }
    }


}
