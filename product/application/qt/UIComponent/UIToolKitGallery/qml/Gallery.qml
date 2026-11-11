import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import UTComponent 1.0
import UIResourceLoader 1.0
import "common"

/**
 * Gallery - showcase window for debugging UIComponent widgets in isolation.
 *
 * Layout: a left tree groups categories by UT module; each category shows
 * its components together on the right. Colors/fonts come
 * from the real ResourceLoader via the "UIResourceLoaderManager" context
 * property, and "切换主题" toggles Light/Dark.
 *
 * All showcase content lives in GalleryCatalog and standalone case files.
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

    property int currentCategory: 0

    GalleryCatalog { id: catalog }

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
            spacing: 12
            UTLabel {
                text: qsTr("Theme: ") + ((GalleryTheme && GalleryTheme.isDark) ? qsTr("Dark") : qsTr("Light"))
                colorEnum: UIColorToken.Content_Heading
            }
            Item { Layout.fillWidth: true }

            UTLabel {
                text: qsTr("Language:")
                colorEnum: UIColorToken.Content_Heading
            }
            UTComboBox {
                Layout.preferredWidth: 140
                textRole: "name"
                valueRole: "code"
                model: ListModel {
                    ListElement { name: "English";  code: "en" }
                    ListElement { name: "中文";      code: "zh_CN" }
                    ListElement { name: "日本語";    code: "ja" }
                    ListElement { name: "한국어";    code: "ko" }
                    ListElement { name: "Français";  code: "fr" }
                    ListElement { name: "Deutsch";   code: "de" }
                }
                onActivated: Qt.uiLanguage = currentValue
            }

            UTButton {
                text: qsTr("切换主题")
                onClicked: GalleryTheme.toggleTheme()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ───────────────────────── Sidebar ─────────────────────────
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 240
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal)

            GalleryNavigation {
                id: sidebar
                anchors.fill: parent
                rows: catalog.navigationRows
                currentPage: window.currentCategory
                onPageSelected: function(page) { window.currentCategory = page }
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

            Repeater {
                model: catalog.pages

                GalleryCategory {
                    required property int index
                    required property var modelData

                    entries: modelData.entries
                    casesActive: window.currentCategory === index
                }
            }
        }
    }
}
