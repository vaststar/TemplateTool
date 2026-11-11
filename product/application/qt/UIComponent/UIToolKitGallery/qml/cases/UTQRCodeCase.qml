import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utQrCodeCase"

    title: "UTQRCode"

    GalleryDemo {
        title: qsTr("Text to QR code")

        UTScrollView {
            id: inputView
            objectName: "qrCodeTextScrollView"
            Layout.fillWidth: true
            Layout.maximumWidth: 520
            Layout.preferredHeight: 98
            contentWidth: availableWidth
            contentHeight: inputContent.implicitHeight

            Item {
                id: inputContent
                width: inputView.availableWidth
                implicitHeight: textInput.height + 8

                UTTextArea {
                    id: textInput
                    objectName: "qrCodeTextInput"
                    x: 4
                    y: 4
                    width: Math.max(0, inputView.availableWidth - 8
                                    - (inputView.ScrollBar.vertical.visible ? inputView.ScrollBar.vertical.width : 0))
                    height: Math.max(90, implicitHeight)
                    text: qsTr("Hello from UIToolKit")
                    placeholderText: qsTr("Enter text to encode...")
                }
            }
        }

        UTCheckBox {
            id: highCorrection
            objectName: "highErrorCorrectionCheckBox"
            text: qsTr("High error correction")
        }

        UTLabel {
            objectName: "qrCodeStatus"
            Layout.fillWidth: true
            text: textInput.text.length === 0
                  ? qsTr("Enter text to generate a QR code.")
                  : standardCode.valid
                    ? qsTr("QR code ready")
                    : qsTr("Unable to generate a QR code. Shorten the text or use medium error correction.")
            wrapMode: Text.Wrap
        }

        Flow {
            Layout.fillWidth: true
            spacing: 24

            Column {
                width: standardCode.implicitWidth
                spacing: 8

                UTLabel {
                    width: parent.width
                    text: qsTr("Default · 200 px")
                    wrapMode: Text.Wrap
                }

                UTQRCode {
                    id: standardCode
                    objectName: "standardQrCode"
                    text: textInput.text
                    errorLevel: highCorrection.checked ? UTQRCodeLevel.High : UTQRCodeLevel.Medium

                    UTLabel {
                        anchors.centerIn: parent
                        width: parent.width
                        visible: textInput.text.length === 0
                        text: qsTr("No content")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }
                }
            }

            Column {
                width: compactCode.implicitWidth
                spacing: 8

                UTLabel {
                    width: parent.width
                    text: qsTr("Compact · 144 px")
                    wrapMode: Text.Wrap
                }

                UTQRCode {
                    id: compactCode
                    objectName: "compactQrCode"
                    displaySize: 144
                    text: textInput.text
                    errorLevel: standardCode.errorLevel

                    UTLabel {
                        anchors.centerIn: parent
                        width: parent.width
                        visible: textInput.text.length === 0
                        text: qsTr("No content")
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}
