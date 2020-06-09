import QtQuick 2.11
import QtQuick.Controls 1.4
import QtQuick.Controls 2.4
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0
import "controls"
import Grimm.Wallet 1.0

Rectangle {
    id: main

    anchors.fill: parent

	MainViewModel {id: viewModel}

    StatusbarViewModel {
        id: statusbarModel
    }
    SettingsViewModel {id: sviewModel}

    gradient: Gradient {
        GradientStop { position: 0.2; color: "#010202" }
        GradientStop { position: 1.0; color: "#101615" }
    }

    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
        propagateComposedEvents: true
        onMouseXChanged: resetLockTimer()
        onPressedChanged: resetLockTimer()
    }

    Keys.onReleased: {
        resetLockTimer()
    }

    property var contentItems : [
		"dashboard",
		"wallet",
		"txs",
    "addresses",
    "verify",
		"donate",
    "utxo",
		"settings"]
    property int selectedItem

    Rectangle {
        id: sidebar
        width: 230
        height: 0
        color: Style.navigation_background
        border.width: 0
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top


        Column {
            width:  0
            height: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 150

            Repeater{
                model: contentItems

                Rectangle {
                    id: control
                    color: Style.navigation_background
                    border.width: 0
                    width: parent.width
                    height: 50
                    activeFocusOnTab: true

                    SvgImage {
						id: icon
                        x: 31
                        y: 4
                        width: 162
                        height: 42
                        source: "qrc:/assets/icon-" + modelData + (selectedItem == index ? "-active" : "") + ".svg"

					}



                    Item {




    					visible: control.activeFocus
                    }
                    Keys.onPressed: {
                        if ((event.key == Qt.Key_Return || event.key == Qt.Key_Enter || event.key == Qt.Key_Space) && selectedItem != index)
                            updateItem(index);
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        onClicked: {
                            control.focus = true
                            if (selectedItem != index)
                                updateItem(index)
                        }
						hoverEnabled: true
                    }
                }



            }
        }

        Image {
            id: image
            y: 5
            anchors.horizontalCenter: parent.horizontalCenter

            source: "qrc:/assets/logo110.svg"

        }



    }

    Loader {
        id: content
        anchors.topMargin: 18
        anchors.bottomMargin: 0
        anchors.rightMargin: 30
        anchors.leftMargin: 250
        anchors.fill: parent
        focus: true
    }

    function updateItem(index)
    {
        selectedItem = index
        content.setSource("qrc:/" + contentItems[index] + ".qml", {"toSend": false})
        viewModel.update(index)
    }

	function openSendDialog() {
		selectedItem = 0
		content.setSource("qrc:/wallet.qml", {"toSend": true})

		viewModel.update(selectedItem)
	}

    function resetLockTimer() {
        viewModel.resetLockTimer();
    }

    Connections {
        target: viewModel
        onGotoStartScreen: {
            main.parent.setSource("qrc:/start.qml", {"isLockedMode": true});
        }
    }

    Component.onCompleted:{
        updateItem(0) // load wallet view by default
    }
}
