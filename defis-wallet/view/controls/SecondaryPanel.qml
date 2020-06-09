import QtQuick 2.11
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.3
import "."

Rectangle {
    id: panel


    property string receiving
    property string sending
    property string maturing
    property string bheight
    property string hash
    property string ver

    radius: 10
    color: Style.navigation_background
    clip: true

    signal copyValueText(string value)

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 15
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.bottomMargin: 15




        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 10


            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                SvgImage {
                    Layout.alignment: Qt.AlignHCenter
                    sourceSize: Qt.size(35, 35)
                    source: "qrc:/assets/ireceive.svg"
                }

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 12
                    font.styleName: "Normal";
                    font.weight: Font.Bold
                    color: Style.content_main
                    opacity: 0.6
                    //% "RECEIVING"
                    text: "RECEIVING"
                }

                SFLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Normal"
                    font.weight: Font.Normal
                    font.pixelSize: 16
                    opacity: 0.6
                    color: Style.accent_incoming
                    text: receiving;
                    elide: Text.ElideRight
                    copyMenuEnabled: true
                    onCopyText: panel.copyValueText(receiving)
                }
            }

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                SvgImage {
                    Layout.alignment: Qt.AlignHCenter
                    sourceSize: Qt.size(35, 35)
                    source: "qrc:/assets/isend.svg"
                }

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 12
                    font.styleName: "Normal";
                    font.weight: Font.Bold
                    color: Style.content_main
                    opacity: 0.6
                    //% "SENDING"
                    text: "SENDING"
                }

                SFLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Normal"
                    font.weight: Font.Normal
                    font.pixelSize: 16
                    opacity: 0.6
                    color: Style.accent_outgoing
                    text: sending;
                    elide: Text.ElideRight
                    copyMenuEnabled: true
                    onCopyText: panel.copyValueText(sending)
                }
            }

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                SvgImage {
                    Layout.alignment: Qt.AlignHCenter
                    sourceSize: Qt.size(35, 35)
                    source: "qrc:/assets/imaturing.svg"
                }

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 12
                    font.styleName: "Normal";
                    font.weight: Font.Bold
                    color: Style.content_main
                    opacity: 0.6
                    //% "MATURING"
                    text: "LOCKED"
                }

                SFLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Normal"
                    font.weight: Font.Normal
                    font.pixelSize: 16
                    opacity: 0.6
                    color: Style.accent_outgoing
                    text: maturing;
                    elide: Text.ElideRight
                    copyMenuEnabled: true
                    onCopyText: panel.copyValueText(maturing)
                }

            }


        }

        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 10


            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                SvgImage {
                    Layout.alignment: Qt.AlignHCenter
                    sourceSize: Qt.size(35, 35)
                    source: "qrc:/assets/iheight.svg"
                }

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 12
                    font.styleName: "Normal";
                    font.weight: Font.Bold
                    color: Style.content_main
                    opacity: 0.6
                    //% "BLOCK HEIGHT"
                    text: "BLOCK HEIGHT"
                }

                SFLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Normal"
                    font.weight: Font.Normal
                    font.pixelSize: 16
                    opacity: 0.6
                    color: Style.accent_incoming
                    text: bheight;
                    elide: Text.ElideRight
                    copyMenuEnabled: true
                    onCopyText: panel.copyValueText(bheight)
                }

            }

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                SvgImage {
                    Layout.alignment: Qt.AlignHCenter
                    sourceSize: Qt.size(35, 35)
                    source: "qrc:/assets/ihash.svg"
                }

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 12
                    font.styleName: "Normal";
                    font.weight: Font.Bold
                    color: Style.content_main
                    opacity: 0.6
                    //% "HASH"
                    text: "LAST BLOCK HASH"
                }

                SFLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Normal"
                    font.weight: Font.Normal
                    font.pixelSize: 16
                    opacity: 0.6
                    color: Style.accent_incoming
                    text: hash;
                    elide: Text.ElideRight
                    copyMenuEnabled: true
                    onCopyText: panel.copyValueText(hash)
                }
            }

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                SvgImage {
                    Layout.alignment: Qt.AlignHCenter
                    sourceSize: Qt.size(40, 35)
                    
                    source: "qrc:/assets/ieye.svg"
                }

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 12
                    font.styleName: "Normal";
                    font.weight: Font.Bold
                    color: Style.content_main
                    opacity: 0.6
                    //% "MATURING"
                    text: "VERSION"
                }

                SFLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Normal"
                    font.weight: Font.Normal
                    font.pixelSize: 16
                    opacity: 0.6
                    color: Style.accent_outgoing
                    text: ver;
                    elide: Text.ElideRight
                    copyMenuEnabled: true
                    onCopyText: panel.copyValueText(maturing)
                }

            }


        }
    }
}
