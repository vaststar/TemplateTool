import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * Gallery - showcase window for debugging UIComponent widgets in isolation.
 *
 * Layout: a left sidebar lists functional categories; clicking one shows only
 * that category's components on the right (switch-style). Colors/fonts come
 * from the real ResourceLoader via the "UIResourceLoaderManager" context
 * property, and "切换主题" toggles Light/Dark.
 *
 * To add a component: drop it into the matching category page below (or add a
 * new entry to `categoryModel` + a new StackLayout page). No C++ changes.
 */
ApplicationWindow {
    id: window
    width: 920
    height: 760
    visible: true
    title: qsTr("UIComponent Gallery")

    // Background re-evaluates when the theme changes.
    property int _rev: UIResourceLoaderManager ? UIResourceLoaderManager.themeRevision : 0
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)

    readonly property int currentCategory: sidebar.currentIndex

    // ── Category list (index order must match the StackLayout pages below) ──
    ListModel {
        id: categoryModel
        ListElement { name: "Buttons" }
        ListElement { name: "Toggles" }
        ListElement { name: "Inputs" }
        ListElement { name: "Pickers" }
        ListElement { name: "Display" }
        ListElement { name: "Containers" }
    }

    header: ToolBar {
        height: 52

        // Themed background so the toolbar follows Light/Dark (otherwise the
        // default Controls style stays white and the heading text disappears
        // in dark theme).
        background: Rectangle {
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            UTLabel {
                text: qsTr("Theme: ") + (GalleryTheme.isDark ? qsTr("Dark") : qsTr("Light"))
                colorEnum: UIColorToken.Content_Heading
            }
            Item { Layout.fillWidth: true }
            UTButton {
                text: qsTr("切换主题")
                onClicked: GalleryTheme.toggleTheme()
            }
        }
    }

    // ── A labelled showcase card. The component's type name sits right next
    //    to the live element, so it is always obvious which UTComponent /
    //    UTComposite widget is being looked at. ──
    component Demo: Rectangle {
        default property alias content: demoRow.data
        property string name: ""   // component type, e.g. "UTButton"
        property string note: ""   // optional variant hint, e.g. "Primary / Disabled"

        Layout.fillWidth: true
        implicitHeight: Math.max(demoId.implicitHeight, demoRow.implicitHeight) + 24
        radius: 8
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 16

            // Identity column — fixed width so every element lines up.
            ColumnLayout {
                id: demoId
                Layout.preferredWidth: 168
                Layout.alignment: Qt.AlignTop
                spacing: 2
                UTLabel {
                    text: name
                    fontEnum: UIFontToken.Body_Text_Medium
                    colorEnum: UIColorToken.Content_Section_Title
                }
                UTLabel {
                    visible: note.length > 0
                    text: note
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
            }

            // Divider between the name and the live element.
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)
            }

            // Live element(s) — the same component, optionally in several states.
            RowLayout {
                id: demoRow
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 16
            }
        }
    }

    // ── Each category page: a heading plus a scrollable column of Demo cards. ──
    component Page: ScrollView {
        default property alias content: pageColumn.data
        property alias title: pageHeading.text
        contentWidth: availableWidth
        clip: true
        ColumnLayout {
            id: pageColumn
            width: window.width - sidebar.width - 48
            spacing: 14
            UTLabel {
                id: pageHeading
                fontEnum: UIFontToken.Heading_Text
                colorEnum: UIColorToken.Content_Heading
                Layout.bottomMargin: 4
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ───────────────────────── Sidebar ─────────────────────────
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal)

            ListView {
                id: sidebar
                anchors.fill: parent
                anchors.topMargin: 12
                model: categoryModel
                currentIndex: 0
                boundsBehavior: Flickable.StopAtBounds

                delegate: ItemDelegate {
                    id: catItem
                    width: ListView.view.width
                    height: 44
                    required property int index
                    required property string name

                    background: Rectangle {
                        color: UTComponentUtil.getPlainUIColor(
                                   UIColorToken.Sidebar_Item_Background,
                                   catItem.index === sidebar.currentIndex ? UIColorState.Selected
                                   : catItem.hovered ? UIColorState.Hovered
                                   : UIColorState.Normal)
                    }

                    contentItem: UTLabel {
                        text: catItem.name
                        leftPadding: 20
                        verticalAlignment: Text.AlignVCenter
                        colorEnum: UIColorToken.Sidebar_Item_Text
                        colorState: catItem.index === sidebar.currentIndex ? UIColorState.Selected : UIColorState.Normal
                    }

                    onClicked: sidebar.currentIndex = catItem.index
                }
            }
        }

        // Divider
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
        }

        // ──────────────────────── Content area ────────────────────────
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            currentIndex: window.currentCategory

            // 0 ─ Buttons
            Page {
                title: qsTr("Buttons")
                Demo {
                    name: "UTButton"
                    note: qsTr("Primary / Disabled")
                    UTButton { text: qsTr("Primary") }
                    UTButton { text: qsTr("Disabled"); enabled: false }
                }
                Demo {
                    name: "UTToolButton"
                    UTToolButton { text: qsTr("Tool") }
                }
            }

            // 1 ─ Toggles
            Page {
                title: qsTr("Toggles")
                Demo {
                    name: "UTCheckBox"
                    UTCheckBox { text: qsTr("Check me"); checked: true }
                    UTCheckBox { text: qsTr("Unchecked"); checked: false }
                }
                Demo {
                    name: "UTSwitch"
                    note: qsTr("On / Off / Disabled")
                    UTSwitch { checked: true }
                    UTSwitch { checked: false }
                    UTSwitch { checked: true; enabled: false }
                }
            }

            // 2 ─ Inputs
            Page {
                title: qsTr("Inputs")
                Demo {
                    name: "UTTextField"
                    UTTextField {
                        Layout.preferredWidth: 320
                        placeholderText: qsTr("Single line...")
                    }
                }
                Demo {
                    name: "UTTextArea"
                    UTTextArea {
                        Layout.preferredWidth: 320
                        Layout.preferredHeight: 90
                        placeholderText: qsTr("Multi-line text...")
                    }
                }
                Demo {
                    name: "UTComboBox"
                    UTComboBox {
                        Layout.preferredWidth: 320
                        model: [qsTr("Option A"), qsTr("Option B"), qsTr("Option C")]
                    }
                }
                Demo {
                    name: "UTSpinBox"
                    UTSpinBox {
                        from: 0; to: 100; value: 4
                    }
                }
            }

            // 3 ─ Pickers
            Page {
                title: qsTr("Pickers")
                Demo {
                    name: "UTTimePicker"
                    UTTimePicker { hours: 9; minutes: 30 }
                }
                Demo {
                    name: "UTDatePicker"
                    UTDatePicker {}
                }
                Demo {
                    name: "UTSlider"
                    UTSlider {
                        Layout.preferredWidth: 320
                        value: 0.4
                    }
                }
            }

            // 4 ─ Display
            Page {
                title: qsTr("Display")
                Demo {
                    name: "UTLabel"
                    note: qsTr("Primary / Secondary")
                    UTLabel { text: qsTr("Body text sample"); colorEnum: UIColorToken.Content_Text }
                    UTLabel { text: qsTr("Secondary text sample"); colorEnum: UIColorToken.Content_Secondary_Text }
                }
                Demo {
                    name: "UTProgressBar"
                    UTProgressBar {
                        Layout.preferredWidth: 320
                        value: 0.65
                    }
                }
                Demo {
                    name: "UTBusyIndicator"
                    UTBusyIndicator { running: true }
                }
            }

            // 5 ─ Containers
            Page {
                title: qsTr("Containers")
                Demo {
                    name: "UTGroupBox"
                    UTGroupBox {
                        Layout.preferredWidth: 320
                        title: qsTr("Settings")
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10
                            UTCheckBox { text: qsTr("Enable feature"); checked: true }
                            UTSwitch { checked: false }
                        }
                    }
                }
            }
        }
    }
}
