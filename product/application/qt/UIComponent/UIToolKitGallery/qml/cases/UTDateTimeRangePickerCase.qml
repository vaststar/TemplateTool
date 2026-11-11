import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UTComposite 1.0
import "../common"

GalleryCase {
    objectName: "utDateTimeRangePickerCase"

    title: "UTDateTimeRangePicker"

    GalleryDemo {
        title: qsTr("Date and time")

        UTScrollView {
            objectName: "dateTimeRangeViewport"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredHeight: contentHeight + effectiveScrollBarHeight
            contentWidth: dateTimeRange.implicitWidth + 8
            contentHeight: dateTimeRange.implicitHeight + 8
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded

            UTDateTimeRangePicker {
                id: dateTimeRange
                objectName: "dateTimeRange"
                x: 4
                y: 4
                width: implicitWidth
                height: implicitHeight
                mode: UTDateTimeRangePicker.DateTime
            }
        }
    }

    GalleryDemo {
        title: qsTr("Date only")

        UTScrollView {
            objectName: "dateRangeViewport"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredHeight: contentHeight + effectiveScrollBarHeight
            contentWidth: dateRange.implicitWidth + 8
            contentHeight: dateRange.implicitHeight + 8
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded

            UTDateTimeRangePicker {
                id: dateRange
                objectName: "dateRange"
                x: 4
                y: 4
                width: implicitWidth
                height: implicitHeight
                mode: UTDateTimeRangePicker.Date
            }
        }
    }

    GalleryDemo {
        title: qsTr("Wheel")

        UTScrollView {
            objectName: "wheelRangeViewport"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredHeight: contentHeight + effectiveScrollBarHeight
            contentWidth: wheelRange.implicitWidth + 8
            contentHeight: wheelRange.implicitHeight + 8
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded

            UTDateTimeRangePicker {
                id: wheelRange
                objectName: "wheelRange"
                x: 4
                y: 4
                width: implicitWidth
                height: implicitHeight
                mode: UTDateTimeRangePicker.DateTime
                datePickerStyle: UTDateTimeRangePicker.Wheel
            }
        }
    }
}
