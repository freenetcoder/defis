import QtQuick 2.11
import QtQuick.Controls 1.2
import QtQuick.Controls 2.4
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import Grimm.Wallet 1.0
import "controls"

Item {
    id: root
    anchors.fill: parent

    WalletViewModel {
        id: viewModel
        onSendMoneyVerified: {
            walletView.enabled = true
            walletView.pop();
        }

        onCantSendToExpired: {
            walletView.enabled = true;
            cantSendToExpiredDialog.open();
        }

        onNewAddressFailed: {
            walletView.enabled = true;
            newAddressFailedDialog.open();
        }
    }

    property bool toSend: false

    Dialog {
        id: cantSendToExpiredDialog

        modal: true

        width: 300
        height: 160

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        visible: false

        background: Rectangle {
            radius: 10
            color: Style.background_second
            anchors.fill: parent
        }

        contentItem: Column {
            anchors.fill: parent
            anchors.margins: 30

            spacing: 40

            SFText {
                anchors.horizontalCenter: parent.horizontalCenter
                //% "Can't send to the expired address."
                text: qsTrId("cant-send-to-expired-message")
                color: Style.content_main
                font.pixelSize: 14
                font.styleName: "Bold"; font.weight: Font.Bold
            }

            PrimaryButton {
                //% "ok"
                text: qsTrId("cant-send-to-expired-ok-button")
                anchors.horizontalCenter: parent.horizontalCenter
                icon.source: "qrc:/assets/icon-done.svg"
                onClicked: cantSendToExpiredDialog.close()
            }
        }
    }

    Dialog {
        id: newAddressFailedDialog

        modal: true

        width: 400
        height: 160

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        visible: false

        background: Rectangle {
            radius: 10
            color: Style.background_second
            anchors.fill: parent
        }

        contentItem: Column {
            anchors.fill: parent
            anchors.margins: 30

            spacing: 40

            SFText {
                width: parent.width
                // text: qsTr("You cannot generate new address. Your wallet doesn't have a master key.")
                //% "You cannot generate new address. Your wallet doesn't have a master key."
                text: qsTrId("can-not-generate-new-address-message")
                color: Style.content_main
                font.pixelSize: 14
                font.styleName: "Bold"; font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }

            PrimaryButton {
                // text: qsTr("ok")
                //% "ok"
                text: qsTrId("can-not-generate-new-address-ok-button")
                anchors.horizontalCenter: parent.horizontalCenter
                icon.source: "qrc:/assets/icon-done.svg"
                onClicked: newAddressFailedDialog.close()
            }
        }
    }

    ConfirmationDialog {
        id: confirmationDialog
        okButtonColor: Style.accent_outgoing
        //% "send"
        okButtonText: qsTrId("send-confirmation-button")
        okButtonIconSource: "qrc:/assets/icon-send-blue.svg"
        cancelButtonIconSource: "qrc:/assets/icon-cancel-white.svg"
        okButtonEnable: viewModel.isPasswordReqiredToSpendMoney() ? requirePasswordInput.text.length : true

        property alias addressText: addressLabel.text
        property alias amountText: amountLabel.text
        property alias feeText: feeLabel.text
        property Item defaultFocusItem: viewModel.isPasswordReqiredToSpendMoney() ? requirePasswordInput : cancelButton

        function confirmationHandler() {
            if (viewModel.isPasswordReqiredToSpendMoney()) {
                if (requirePasswordInput.text.length == 0) {
                    requirePasswordInput.forceActiveFocus(Qt.TabFocusReason);
                    return;
                }
                if (!viewModel.isPasswordValid(requirePasswordInput.text)) {
                    requirePasswordInput.forceActiveFocus(Qt.TabFocusReason);
                    //% "Invalid password provided."
                    requirePasswordError.text = qsTrId("send-confirmation-pwd-fail");
                    return;
                }
            }
            accepted();
            close();
        }

        function openHandler() {
            var defaultFocusItem = viewModel.isPasswordReqiredToSpendMoney() ? requirePasswordInput : cancelButton;
            defaultFocusItem.forceActiveFocus(Qt.TabFocusReason);
        }

        function requirePasswordInputKeyEnter() {
            okButton.forceActiveFocus(Qt.TabFocusReason);
            okButton.clicked();
        }

        contentItem: Item {
            id: sendConfirmationContent
            ColumnLayout {
                anchors.fill: parent
                spacing: 30

                SFText {
                    id: title
                    Layout.alignment: Qt.AlignHCenter
                    Layout.minimumHeight: 21
                    Layout.leftMargin: 68
                    Layout.rightMargin: 68
                    Layout.topMargin: 14
                    font.pixelSize: 18
                    font.styleName: "Bold";
                    font.weight: Font.Bold
                    color: Style.content_main
                    //% "Confirm transaction details"
                    text: qsTrId("send-confirmation-title")
                }

                GridLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 18
                    Layout.rightMargin: 18
                    columnSpacing: 14
                    rowSpacing: 12
                    columns: 2
                    rows: 5

                    SFText {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 16
                        font.pixelSize: 14
                        color: Style.content_disabled
                        //% "Recipient:"
                        text: qsTrId("send-confirmation-recipient-label")
                        verticalAlignment: Text.AlignTop
                    }

                    SFText {
                        id: addressLabel
                        Layout.fillWidth: true
                        Layout.maximumWidth: 290
                        Layout.minimumHeight: 16
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        font.pixelSize: 14
                        color: Style.content_main
                    }

                    SFText {
                        Layout.row: 2
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 16
                        Layout.bottomMargin: 3
                        font.pixelSize: 14
                        color: Style.content_disabled
                        //% "Amount:"
                        text: qsTrId("send-confirmation-amount-label")
                        verticalAlignment: Text.AlignBottom
                    }

                    SFText {
                        id: amountLabel
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 29
                        font.pixelSize: 24
                        color: Style.accent_outgoing
                        verticalAlignment: Text.AlignBottom
                    }

                    SFText {
                        Layout.row: 3
                        Layout.fillWidth: true
                        Layout.minimumHeight: 16
                        font.pixelSize: 14
                        color: Style.content_disabled
                        //% "Transaction fee:"
                        text: qsTrId("send-confirmation-fee-label")
                    }

                    SFText {
                        id: feeLabel
                        Layout.fillWidth: true
                        Layout.minimumHeight: 16
                        font.pixelSize: 14
                        color: Style.content_main
                    }

                    SFText {
                        id: requirePasswordLabel
                        visible: viewModel.isPasswordReqiredToSpendMoney()
                        Layout.row: 4
                        Layout.columnSpan: 2
                        Layout.topMargin: 50
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.minimumHeight: 16
                        font.pixelSize: 14
                        color: Style.content_main
                        //% "To broadcast your transaction please enter your password"
                        text: qsTrId("send-confirmation-pwd-require-message")
                    }

                    SFTextInput {
                        id: requirePasswordInput
                        visible: viewModel.isPasswordReqiredToSpendMoney()
                        Layout.row: 5
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        focus: true
                        activeFocusOnTab: true
                        font.pixelSize: 14
                        color: Style.content_main
                        echoMode: TextInput.Password
                        onAccepted: confirmationDialog.requirePasswordInputKeyEnter()
                        onTextChanged: if (requirePasswordError.text.length > 0) requirePasswordError.text = ""

                    }
                    SFText {
                        id: requirePasswordError
                        visible: viewModel.isPasswordReqiredToSpendMoney()
                        Layout.row: 6
                        Layout.columnSpan: 2
                        Layout.bottomMargin: 15
                        height: 16
                        width: parent.width
                        color: Style.validator_error
                        font.pixelSize: 14
                    }
                }
            }
        }

        onClosed: {
            requirePasswordInput.text = "";
        }

        onAccepted: {
            viewModel.sendMoney();
            walletView.enabled = false;
            root.parent.source = "qrc:/txs.qml";
        }
    }

    ConfirmationDialog {
        id: invalidAddressDialog
        //% "got it"
        okButtonText: qsTrId("invalid-addr-got-it-button")
    }

    ConfirmationDialog {
        id: deleteTransactionDialog
        //% "delete"
        okButtonText: qsTrId("delete-transaction-delete-button")
    }

    OpenExternalLinkConfirmation {
        id: exchangesList
    }

    PaymentInfoDialog {
        id: paymentInfoDialog
        onTextCopied: function(text){
            viewModel.copyToClipboard(text);
        }
    }

    PaymentInfoItem {
        id: verifyInfo
    }

    PaymentInfoDialog {
        id: paymentInfoVerifyDialog
        shouldVerify: true

        model:verifyInfo
        onTextCopied: function(text){
            viewModel.copyToClipboard(text);
        }
    }

    Item {

        height: 110

        anchors.left: parent.left
        anchors.right: parent.right

        RowLayout {



            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height




            DashboardPanel {
                  SettingsViewModel {id: sviewModel}
                  UtxoViewModel {id: uviewModel}
                  Layout.minimumWidth: 350
                  Layout.fillHeight: true
                  Layout.fillWidth: true
                  bheight: uviewModel.currentHeight
                  ver: sviewModel.version
                  receiving: viewModel.receiving
                  sending: viewModel.sending
                  maturing: viewModel.maturing
                  balance: viewModel.available


              }



        }
    }


    /////////////////////////////////////////////////////////////
    /// Receive layout //////////////////////////////////////////
    /////////////////////////////////////////////////////////////

    Component {
        id: receive_layout
        Item {

            property Item defaultFocusItem: myAddressName
            property bool isAddressCommentDuplicated: false

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 73
                anchors.bottomMargin: 30
                spacing: 30

                SFText {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.minimumHeight: 21
                    font.pixelSize: 18
                    font.styleName: "Bold"; font.weight: Font.Bold
                    color: Style.content_main
                    //% "Receive Grimm"
                    text: qsTrId("wallet-receive-title")
                }

                RowLayout {
                    Layout.fillWidth: true

                    Item {
                        Layout.fillWidth: true
                        // TODO: find better solution, because it's bad
                        Layout.minimumHeight: 220
                        Column {
                            anchors.fill: parent
                            spacing: 10

                            SFText {
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                                color: Style.content_main
                                //% "My address"
                                text: qsTrId("wallet-receive-my-addr-label")
                            }

                            SFTextInput {
                                id: myAddressID
                                width: parent.width
                                font.pixelSize: 14
                                color: Style.content_disabled
                                readOnly: true
                                activeFocusOnTab: false
                                text: viewModel.newReceiverAddr
                            }

                            Row {
                                spacing: 10
                                SFText {
                                    font.pixelSize: 14
                                    font.italic: true
                                    color: Style.content_main
                                    //% "Expires:"
                                    text: qsTrId("wallet-receive-expires-label")
                                }
                                CustomComboBox {
                                    id: expiresControl
                                    width: 100
                                    height: 20
                                    anchors.top: parent.top
                                    anchors.topMargin: -3

                                    currentIndex: viewModel.expires

                                    Binding {
                                        target: viewModel
                                        property: "expires"
                                        value: expiresControl.currentIndex
                                    }

                                    model: [
                                        //% "24 hours"
                                        qsTrId("wallet-receive-expires-24"),
                                        //% "never"
                                        qsTrId("wallet-receive-expires-never")
                                    ]
                                }
                            }


                            // Comment
                            SFText {
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                                color: Style.content_main
                                //% "Comment"
                                text: qsTrId("wallet-receive-comment-label")
                            }

                            SFTextInput {
                                id: myAddressName
                                font.pixelSize: 14
                                width: parent.width
                                font.italic : isAddressCommentDuplicated
                                backgroundColor: isAddressCommentDuplicated ? Style.validator_error : Style.content_main
                                color: isAddressCommentDuplicated ? Style.validator_error : Style.content_main
                                focus: true
                                text: viewModel.newReceiverName
                                onTextEdited: {
                                    isAddressCommentDuplicated = viewModel.isAddressWithCommentExist(myAddressName.text);
                                    if (!isAddressCommentDuplicated) {
                                        viewModel.newReceiverName = myAddressName.text;
                                    }
                                }
                            }

                            SFText {
                                //: Create address, address with same comment already exist error
                                //% "Address with same comment already exist"
                                text: qsTrId("create-addr-comment-error")
                                color: Style.validator_error
                                font.pixelSize: 12
                                visible: isAddressCommentDuplicated
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        // TODO: find better solution, because it's bad
                        Layout.minimumHeight: 220
                        Column {
                            anchors.fill: parent
                            Image {
                                anchors.horizontalCenter: parent.horizontalCenter
                                fillMode: Image.Pad

                                source: viewModel.newReceiverAddrQR
                            }
                            SFText {
                                anchors.horizontalCenter: parent.horizontalCenter
                                font.pixelSize: 14
                                font.italic: true
                                color: Style.content_main
                                //% "Scan to send"
                                text: qsTrId("wallet-receive-qr-label")
                            }
                        }
                    }
                }

                SFText {
                    Layout.topMargin: 15
                    Layout.alignment: Qt.AlignHCenter
                    Layout.minimumHeight: 16
                    font.pixelSize: 14
                    color: Style.content_main
                    //% "Send this address to the sender over an external secure channel"
                    text: qsTrId("wallet-receive-propogate-addr-message")
                }
                Row {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.minimumHeight: 40

                    spacing: 19

                    CustomButton {
                        //% "close"
                        text: qsTrId("wallet-receive-close-button")
                        palette.buttonText: Style.content_main
                        icon.source: "qrc:/assets/icon-cancel-white.svg"
                        onClicked: {
                            walletView.pop();
                        }
                    }

                    CustomButton {
                        //% "copy"
                        text: qsTrId("wallet-receive-copy-button")
                        palette.buttonText: Style.content_opposite
                        icon.color: Style.content_opposite
                        palette.button: Style.active
                        icon.source: "qrc:/assets/icon-copy.svg"
                        onClicked: {
                            viewModel.copyToClipboard(myAddressID.text);
                        }
                    }
                }

                Component.onDestruction: {
                    // TODO: "Save" may be deleted in future, when we'll have editor for own addresses.
                    if (!isAddressCommentDuplicated) {
                        viewModel.saveNewAddress();
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////
    /// Send layout /////////////////////////////////////////////
    /////////////////////////////////////////////////////////////

    Component {
        id: wallet_layout
        Item {

            y: 40
            property Item defaultFocusItem: receiverAddrInput

            Component.onCompleted: {
                receiverAddrInput.text = "";
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 73
                anchors.bottomMargin: 30



                Item {
                    Layout.fillHeight: true
                    Layout.minimumHeight: 10
                    Layout.maximumHeight: 30
                }

                RowLayout {
                    Layout.fillWidth: true
                    //Layout.topMargin: 50

                    spacing: 70

                    Item {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        height: childrenRect.height

                        ColumnLayout {
                            width: parent.width

                            spacing: 12

                            SFText {
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                                color: Style.content_main
                                //% "Send To:"
                                text: qsTrId("send-send-to-label")
                            }

                            SFTextInput {
                                Layout.fillWidth: true
                                Layout.topMargin: 25
                                id: receiverAddrInput
                                font.pixelSize: 14
                                color: Style.content_main
                                text: viewModel.receiverAddr

                                validator: RegExpValidator { regExp: /[0-9a-fA-F]{1,80}/ }
                                selectByMouse: true

                                //% "Please specify contact"
                                placeholderText: qsTrId("send-contact-placeholder")

                                onTextChanged : {
                                    receiverAddressError.visible = receiverAddrInput.text.length > 0 && !viewModel.isValidReceiverAddress(receiverAddrInput.text)
                                }
                            }

                            SFText {
                                Layout.alignment: Qt.AlignTop
                                id: receiverAddressError
                                color: Style.validator_error
                                font.pixelSize: 10
                                //% "Invalid address"
                                text: qsTrId("send-addr-fail")
                                visible: false
                            }

                            SFText {
                                id: receiverName
                                color: Style.content_main
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                            }

                            Binding {
                                target: viewModel
                                property: "receiverAddr"
                                value: receiverAddrInput.text
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        height: childrenRect.height

                        ColumnLayout {
                            width: parent.width

                            spacing: 12

                            SFText {
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                                color: Style.content_main
                                //% "Transaction amount"
                                text: qsTrId("send-amount-label")
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                SFTextInput {
                                    Layout.fillWidth: true

                                    id: amount_input

                                    font.pixelSize: 36
                                    font.styleName: "Light"; font.weight: Font.Light
                                    color: Style.accent_outgoing

                                    property double amount: 0

                                    validator: RegExpValidator { regExp: /^(([1-9][0-9]{0,7})|(1[0-9]{8})|(2[0-4][0-9]{7})|(25[0-3][0-9]{6})|(0))(\.[0-9]{0,7}[1-9])?$/ }
                                    selectByMouse: true

                                    onTextChanged: {
                                        if (focus) {
                                            amount = text ? text : 0;
                                        }
                                    }

                                    onFocusChanged: {
                                        if (amount > 0) {
                                            text = amount.toLocaleString(focus ? Qt.locale("C") : Qt.locale(), 'f', -128);
                                        }
                                    }
                                }

                                Binding {
                                    target: viewModel
                                    property: "sendAmount"
                                    value: amount_input.amount
                                }

                                SFText {
                                    font.pixelSize: 24
                                    color: Style.content_main
                                    //% "XGM"
                                    text: qsTrId("send-curency-name")
                                }
                            }
                            Item {
                                Layout.topMargin: -12
                                Layout.minimumHeight: 16
                                Layout.fillWidth: true

                                SFText {
                                    //% "Insufficient funds: you would need %1 to complete the transaction"
                                    text: qsTrId("send-founds-fail").arg(viewModel.amountMissingToSend)
                                    color: Style.validator_error
                                    font.pixelSize: 14
                                    fontSizeMode: Text.Fit
                                    minimumPixelSize: 10
                                    font.styleName: "Italic"
                                    width: parent.width
                                    visible: !viewModel.isEnoughMoney
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    spacing: 70

                    Item {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        height: childrenRect.height

                        ColumnLayout {
                            width: parent.width

                            spacing: 12

                            SFText {
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                                color: Style.content_main
                                //% "Comment"
                                text: qsTrId("send-comment-label")
                            }

                            SFTextInput {
                                id: comment_input
                                Layout.fillWidth: true
                                Layout.topMargin: 25

                                font.pixelSize: 14
                                color: Style.content_main

                                maximumLength: 1024
                                selectByMouse: true
                            }

                            Binding {
                                target: viewModel
                                property: "comment"
                                value: comment_input.text
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        height: childrenRect.height

                        ColumnLayout {
                            width: parent.width

                            spacing: 12

                            SFText {
                                font.pixelSize: 14
                                font.styleName: "Bold"; font.weight: Font.Bold
                                color: Style.content_main
                                //% "Transaction fee"
                                text: qsTrId("send-fee-label")
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                ColumnLayout {
                                    Layout.fillWidth: true

                                    SFTextInput {
                                        Layout.fillWidth: true
                                        id: fee_input

                                        font.pixelSize: 36
                                        font.styleName: "Light"; font.weight: Font.Light
                                        color: Style.accent_outgoing

                                        text: viewModel.defaultFeeInCentum.toLocaleString(Qt.locale(), 'f', -128)

                                        property int amount: viewModel.defaultFeeInCentum

                                        validator: IntValidator {bottom: viewModel.minimumFeeInCentum}
                                        maximumLength: 15
                                        selectByMouse: true

                                        onTextChanged: {
                                            if (focus) {
                                                amount = text ? text : 0;
                                            }
                                        }

                                        onFocusChanged: {
                                            if (amount >= 0) {
                                                // QLocale::FloatingPointShortest = -128
                                                text = focus ? amount : amount.toLocaleString(Qt.locale(), 'f', -128);
                                            }
                                        }
                                    }
                                }

                                SFText {
                                    font.pixelSize: 24
                                    color: Style.content_main
                                    //% "CENTUM"
                                    text: qsTrId("send-curency-sub-name")
                                }
                            }

                            Item {
                                Layout.topMargin: -12
                                Layout.minimumHeight: 16
                                Layout.fillWidth: true

                                SFText {
                                    //% "The minimum fee is %1 centum"
                                    text: qsTrId("send-fee-fail").arg(viewModel.minimumFeeInCentum)
                                    color: Style.validator_error
                                    font.pixelSize: 14
                                    fontSizeMode: Text.Fit
                                    minimumPixelSize: 10
                                    font.styleName: "Italic"
                                    width: parent.width
                                    visible: fee_input.amount < viewModel.minimumFeeInCentum
                                }
                            }

                            Binding {
                                target: viewModel
                                property: "feeCentumes"
                                //value: feeSlider.value
                                value: fee_input.amount
                            }


                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 20
                    height: 96


                    Item {
                        anchors.fill: parent

                        RowLayout {
                            anchors.fill: parent
                            readonly property int margin: 15
                            anchors.leftMargin: margin
                            anchors.rightMargin: margin
                            spacing: margin

                            Item {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignCenter
                                height: childrenRect.height

                                ColumnLayout {
                                    width: parent.width
                                    spacing: 10

                                    SFText {
                                        Layout.alignment: Qt.AlignHCenter
                                        font.pixelSize: 18
                                        font.styleName: "Bold"; font.weight: Font.Bold
                                        color: Style.content_secondary
                                        //% "Remaining"
                                        text: qsTrId("send-remaining-label")
                                    }

                                    RowLayout
                                    {
                                        Layout.alignment: Qt.AlignHCenter
                                        spacing: 6
                                        clip: true

                                        SFText {
                                            font.pixelSize: 24
                                            font.styleName: "Light"; font.weight: Font.Light
                                            color: Style.content_secondary
                                            text: viewModel.actualAvailable
                                        }

                                        SvgImage {
                                            Layout.topMargin: 4
                                            sourceSize: Qt.size(16, 24)
                                            source: "qrc:/assets/b-grey.svg"
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                id: separator
                                Layout.fillHeight: true
                                Layout.topMargin: 10
                                Layout.bottomMargin: 10
                                width: 1
                                color: Style.content_secondary
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignCenter
                                height: childrenRect.height

                                ColumnLayout {
                                    width: parent.width
                                    spacing: 10

                                    SFText {
                                        Layout.alignment: Qt.AlignHCenter
                                        font.pixelSize: 18
                                        font.styleName: "Bold"; font.weight: Font.Bold
                                        color: Style.content_secondary
                                        //% "Change"
                                        text: qsTrId("send-change-label")
                                    }

                                    RowLayout
                                    {
                                        Layout.alignment: Qt.AlignHCenter
                                        spacing: 6
                                        clip: true

                                        SFText {
                                            font.pixelSize: 24
                                            font.styleName: "Light"; font.weight: Font.Light
                                            color: Style.content_secondary
                                            text: viewModel.change
                                        }

                                        SvgImage {
                                            Layout.topMargin: 4
                                            sourceSize: Qt.size(16, 24)
                                            source: "qrc:/assets/b-grey.svg"
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        radius: 10
                        color: Style.white
                        opacity: 0.1
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.minimumHeight: 10
                    Layout.maximumHeight: 30
                }

                Row {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 30



                    CustomButton {
                        //% "send"
                        text: qsTrId("send-send-button")
                        palette.buttonText: Style.content_opposite
                        palette.button: Style.accent_outgoing
                        icon.source: "qrc:/assets/icon-send-blue.svg"
                        enabled: {viewModel.isEnoughMoney && amount_input.amount > 0 && receiverAddrInput.acceptableInput }
                        onClicked: {
                            if (viewModel.isValidReceiverAddress(viewModel.receiverAddr)) {
                                confirmationDialog.addressText = viewModel.receiverAddr;
                                //% "XGM"
                                confirmationDialog.amountText = amount_input.amount.toLocaleString(Qt.locale(), 'f', -128) + " " + qsTrId("send-curency-name");
                                //% "CENTUM"
                                confirmationDialog.feeText = fee_input.amount.toLocaleString(Qt.locale(), 'f', -128) + " " + qsTrId("send-curency-sub-name");

                                confirmationDialog.open();
                            } else {
                                //% "Address %1 is invalid"
                                var message = qsTrId("send-send-fail");
                                invalidAddressDialog.text = message.arg(viewModel.receiverAddr);
                                invalidAddressDialog.open();
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }





    StackView {
        id: walletView
        anchors.fill: parent
        initialItem: wallet_layout

        pushEnter: Transition {
            enabled: false
        }
        pushExit: Transition {
            enabled: false
        }
        popEnter: Transition {
            enabled: false
        }
        popExit: Transition {
            enabled: false
        }

        onCurrentItemChanged: {
            if (currentItem && currentItem.defaultFocusItem) {
                walletView.currentItem.defaultFocusItem.forceActiveFocus();
            }
        }
    }

    Component.onCompleted: {
        if (root.toSend) {
            walletView.push(send_layout);
            root.toSend = false;
        }
    }
}
