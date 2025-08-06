import QtQuick.Controls

Dialog {
    id: dialog
    title: "myTitle"
    modal: true
    width: 300
    height: 300
    standardButtons: Dialog.Ok | Dialog.Cancel
    closePolicy:Popup.NoAutoClose
    anchors.centerIn: parent

    onAccepted: console.log("Ok clicked")
    onRejected: console.log("Cancel clicked")
}