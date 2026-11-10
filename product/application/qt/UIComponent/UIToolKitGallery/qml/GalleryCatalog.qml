import QtQuick

QtObject {
    readonly property var entries: [
        {
            id: "button",
            category: "buttons",
            title: "UTButton",
            source: Qt.resolvedUrl("cases/UTButtonCase.qml"),
            coveredTypes: ["UTButton"]
        },
        {
            id: "toolButton",
            category: "buttons",
            title: "UTToolButton",
            source: Qt.resolvedUrl("cases/UTToolButtonCase.qml"),
            coveredTypes: ["UTToolButton"]
        },
        {
            id: "checkBox",
            category: "toggles",
            title: "UTCheckBox",
            source: Qt.resolvedUrl("cases/UTCheckBoxCase.qml"),
            coveredTypes: ["UTCheckBox"]
        },
        {
            id: "switch",
            category: "toggles",
            title: "UTSwitch",
            source: Qt.resolvedUrl("cases/UTSwitchCase.qml"),
            coveredTypes: ["UTSwitch"]
        }
    ]
}
