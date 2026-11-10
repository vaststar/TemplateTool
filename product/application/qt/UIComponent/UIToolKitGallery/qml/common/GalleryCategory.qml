import QtQuick
import QtQuick.Layouts
import UTComponent 1.0

UTScrollView {
    id: root

    property var entries: []
    property bool casesActive: true

    ColumnLayout {
        width: root.availableWidth
        spacing: 24

        Repeater {
            model: root.entries

            ColumnLayout {
                id: caseEntry
                required property var modelData
                Layout.fillWidth: true

                Loader {
                    id: caseLoader
                    Layout.fillWidth: true
                    Layout.preferredHeight: item ? item.implicitHeight : 0
                    active: root.casesActive
                    source: caseEntry.modelData.source
                }
                UTLabel {
                    Layout.fillWidth: true
                    visible: caseLoader.status === Loader.Error
                    text: qsTr("Failed to load %1.").arg(caseEntry.modelData.title)
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
