import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0

RowLayout {
    id: root

    required property PermissionController controller
    required property string message

    spacing: 12
    visible: root.controller.initialized
             && (root.controller.needsGuidance
                 || root.controller.operationError !== PermissionController.NoError)

    UTText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideNone
        text: {
            switch (root.controller.operationError) {
            case PermissionController.UnsupportedOperation:
                return qsTr("This permission operation is not supported on this platform.")
            case PermissionController.QueryFailed:
                return qsTr("Unable to check permission.")
            case PermissionController.RequestFailed:
                return qsTr("Unable to request permission. Open System Settings to review access.")
            case PermissionController.SettingsOpenFailed:
                return qsTr("Unable to open System Settings. Open Privacy & Security manually.")
            }
            if (root.controller.status === PermissionController.Restricted)
                return qsTr("Access is restricted by system policy.")
            return root.message
        }
    }

    UTButton {
        visible: root.controller.action !== PermissionController.NoAction
        enabled: !root.controller.busy
        text: root.controller.busy ? qsTr("Please wait...")
              : root.controller.action === PermissionController.Request
                ? qsTr("Allow Access") : qsTr("Open System Settings")
        onClicked: root.controller.performAction()
    }
}
