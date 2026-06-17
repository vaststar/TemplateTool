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

    // ── Reusable section header inside a category page ──
    component Section: ColumnLayout {
        property alias title: header.text
        Layout.fillWidth: true
        spacing: 10
        UTLabel {
            id: header
            fontEnum: UIFontToken.Body_Text_Medium
            colorEnum: UIColorToken.Content_Heading
        }
    }

    // ── Each category page is a scrollable column ──
    component Page: ScrollView {
        default property alias content: pageColumn.data
        contentWidth: availableWidth
        clip: true
        ColumnLayout {
            id: pageColumn
            width: window.width - sidebar.width - 48
            spacing: 28
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
                Section {
                    title: qsTr("Buttons")
                    RowLayout {
                        spacing: 12
                        UTButton { text: qsTr("Primary") }
                        UTButton { text: qsTr("Disabled"); enabled: false }
                        UTToolButton { text: qsTr("Tool") }
                    }
                }
            }

            // 1 ─ Toggles
            Page {
                Section {
                    title: qsTr("Check / Switch")
                    RowLayout {
                        spacing: 24
                        UTCheckBox { text: qsTr("Check me"); checked: true }
                        UTSwitch { checked: true }
                        UTSwitch { checked: false }
                        UTSwitch { checked: true; enabled: false }
                    }
                }
            }

            // 2 ─ Inputs
            Page {
                Section {
                    title: qsTr("Text fields")
                    UTTextField {
                        Layout.preferredWidth: 340
                        placeholderText: qsTr("Single line...")
                    }
                    UTTextArea {
                        Layout.preferredWidth: 340
                        Layout.preferredHeight: 90
                        placeholderText: qsTr("Multi-line text...")
                    }
                }
                Section {
                    title: qsTr("Selection / Number")
                    UTComboBox {
                        Layout.preferredWidth: 340
                        model: [qsTr("Option A"), qsTr("Option B"), qsTr("Option C")]
                    }
                    UTSpinBox {
                        from: 0; to: 100; value: 4
                    }
                }
            }

            // 3 ─ Pickers
            Page {
                Section {
                    title: qsTr("Time picker")
                    UTTimePicker { hours: 9; minutes: 30 }
                }
                Section {
                    title: qsTr("Date picker")
                    UTDatePicker {}
                }
                Section {
                    title: qsTr("Slider")
                    UTSlider {
                        Layout.preferredWidth: 340
                        value: 0.4
                    }
                }
            }

            // 4 ─ Display
            Page {
                Section {
                    title: qsTr("Labels")
                    UTLabel { text: qsTr("Body text sample"); colorEnum: UIColorToken.Content_Text }
                    UTLabel { text: qsTr("Secondary text sample"); colorEnum: UIColorToken.Content_Secondary_Text }
                }
                Section {
                    title: qsTr("Progress / Busy")
                    UTProgressBar {
                        Layout.preferredWidth: 340
                        value: 0.65
                    }
                    UTBusyIndicator { running: true }
                }
            }

            // 5 ─ Containers
            Page {
                Section {
                    title: qsTr("Group box")
                    UTGroupBox {
                        Layout.preferredWidth: 360
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
