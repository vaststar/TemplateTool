import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    title: "myTitle"
    modal: false
    // modality: Qt.ApplicationModal
    standardButtons: Dialog.Ok | Dialog.Cancel
    closePolicy:Popup.NoAutoClose
    anchors.centerIn: parent

    onAccepted: console.log("Ok clicked")
    onRejected: console.log("Cancel clicked")
}