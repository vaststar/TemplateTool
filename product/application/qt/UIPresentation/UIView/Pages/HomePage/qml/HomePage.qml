import QtQuick
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: homePage
    property HomePageController controller: HomePageController {}

    Column {
        anchors.centerIn: parent
        spacing: 16

        UTText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "首页"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        UTLabel {
            id: linkLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: '点击访问 <a href="https://www.qt.io">Qt 官网</a> 了解更多'
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
            linkColorEnum: UIColorToken.Link_Text
            activeFocusOnTab: true
            onLinkActivated: (link) => Qt.openUrlExternally(link)
        }

        UTButton {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200
            height: 36
            text: qsTr("Show Test Message")
            appearance: UTButton.Secondary
            onClicked: homePage.controller.showTestMessage()
        }

        UTButton {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200
            height: 36
            text: qsTr("Open Camera")
            appearance: UTButton.Secondary
            onClicked: homePage.controller.openCamera()
        }

        UTButton {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200
            height: 36
            text: qsTr("Open Camera Monitor")
            appearance: UTButton.Secondary
            onClicked: homePage.controller.openCameraMonitor()
        }

        UTTimePicker {
            id: timePicker
            anchors.horizontalCenter: parent.horizontalCenter
            hours: 9
            minutes: 30
            showSeconds: true
        }

        UTText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Selected time: ") + timePicker.timeValue
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        UTDatePicker {
            id: datePicker
            anchors.horizontalCenter: parent.horizontalCenter
            // 可选范围：过去 10 年 ~ 未来 10 年
            minDate: new Date(new Date().getFullYear() - 10, 0, 1)
            maxDate: new Date(new Date().getFullYear() + 10, 11, 31)
        }

        UTText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Selected date: ") + datePicker.dateValue
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }
    }
}
