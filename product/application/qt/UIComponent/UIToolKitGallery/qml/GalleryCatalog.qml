import QtQuick

QtObject {
    readonly property var modules: [
        {
            title: "UTComponent",
            categories: [
                {
                    title: "Buttons",
                    entries: [
                        {
                            id: "button",
                            title: "UTButton",
                            source: Qt.resolvedUrl("cases/UTButtonCase.qml"),
                            coveredTypes: ["UTButton"]
                        },
                        {
                            id: "toolButton",
                            title: "UTToolButton",
                            source: Qt.resolvedUrl("cases/UTToolButtonCase.qml"),
                            coveredTypes: ["UTToolButton"]
                        }
                    ]
                },
                {
                    title: "Toggles",
                    entries: [
                        {
                            id: "checkBox",
                            title: "UTCheckBox",
                            source: Qt.resolvedUrl("cases/UTCheckBoxCase.qml"),
                            coveredTypes: ["UTCheckBox"]
                        },
                        {
                            id: "switch",
                            title: "UTSwitch",
                            source: Qt.resolvedUrl("cases/UTSwitchCase.qml"),
                            coveredTypes: ["UTSwitch"]
                        }
                    ]
                },
                {
                    title: "Inputs",
                    entries: [
                        {
                            id: "spinBox",
                            title: "UTSpinBox",
                            source: Qt.resolvedUrl("cases/UTSpinBoxCase.qml"),
                            coveredTypes: ["UTSpinBox"]
                        },
                        {
                            id: "textField",
                            title: "UTTextField",
                            source: Qt.resolvedUrl("cases/UTTextFieldCase.qml"),
                            coveredTypes: ["UTTextField"]
                        },
                        {
                            id: "textArea",
                            title: "UTTextArea",
                            source: Qt.resolvedUrl("cases/UTTextAreaCase.qml"),
                            coveredTypes: ["UTTextArea"]
                        },
                        {
                            id: "comboBox",
                            title: "UTComboBox",
                            source: Qt.resolvedUrl("cases/UTComboBoxCase.qml"),
                            coveredTypes: ["UTComboBox"]
                        }
                    ]
                },
                {
                    title: "Pickers",
                    entries: [
                        {
                            id: "timePicker",
                            title: "UTTimePicker",
                            source: Qt.resolvedUrl("cases/UTTimePickerCase.qml"),
                            coveredTypes: ["UTTimePicker"]
                        },
                        {
                            id: "datePicker",
                            title: "UTDatePicker",
                            source: Qt.resolvedUrl("cases/UTDatePickerCase.qml"),
                            coveredTypes: ["UTDatePicker"]
                        },
                        {
                            id: "dateWheelPicker",
                            title: "UTDateWheelPicker",
                            source: Qt.resolvedUrl("cases/UTDateWheelPickerCase.qml"),
                            coveredTypes: ["UTDateWheelPicker"]
                        },
                        {
                            id: "slider",
                            title: "UTSlider",
                            source: Qt.resolvedUrl("cases/UTSliderCase.qml"),
                            coveredTypes: ["UTSlider"]
                        }
                    ]
                },
                {
                    title: "Display",
                    entries: [
                        {
                            id: "label",
                            title: "UTLabel",
                            source: Qt.resolvedUrl("cases/UTLabelCase.qml"),
                            coveredTypes: ["UTLabel"]
                        },
                        {
                            id: "text",
                            title: "UTText",
                            source: Qt.resolvedUrl("cases/UTTextCase.qml"),
                            coveredTypes: ["UTText"]
                        },
                        {
                            id: "contentDivider",
                            title: "UTContentDivider",
                            source: Qt.resolvedUrl("cases/UTContentDividerCase.qml"),
                            coveredTypes: ["UTContentDivider"]
                        },
                        {
                            id: "progressBar",
                            title: "UTProgressBar",
                            source: Qt.resolvedUrl("cases/UTProgressBarCase.qml"),
                            coveredTypes: ["UTProgressBar"]
                        },
                        {
                            id: "busyIndicator",
                            title: "UTBusyIndicator",
                            source: Qt.resolvedUrl("cases/UTBusyIndicatorCase.qml"),
                            coveredTypes: ["UTBusyIndicator"]
                        }
                    ]
                },
                {
                    title: "Containers",
                    entries: [
                        {
                            id: "groupBox",
                            title: "UTGroupBox",
                            source: Qt.resolvedUrl("cases/UTGroupBoxCase.qml"),
                            coveredTypes: ["UTGroupBox"]
                        }
                    ]
                }
            ]
        },
        {
            title: "UTComposite",
            categories: [
                {
                    title: "Pickers",
                    entries: [
                        {
                            id: "dateTimeRangePicker",
                            title: "UTDateTimeRangePicker",
                            source: Qt.resolvedUrl("cases/UTDateTimeRangePickerCase.qml"),
                            coveredTypes: ["UTDateTimeRangePicker"]
                        }
                    ]
                }
            ]
        }
    ]

    readonly property var pages: {
        const result = []
        for (const module of modules) {
            for (const category of module.categories)
                result.push(category)
        }
        return result
    }

    readonly property var navigationRows: {
        let page = 0
        return modules.map(module => ({
            title: module.title,
            page: -1,
            rows: module.categories.map(category => ({
                title: category.title,
                page: page++
            }))
        }))
    }
}
