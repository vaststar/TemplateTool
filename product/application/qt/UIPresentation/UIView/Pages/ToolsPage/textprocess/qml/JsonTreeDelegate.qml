import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/// Delegate for rendering a single row in the JSON tree view.
/// nodeType values: 0=Object, 1=Array, 2=String, 3=Number, 4=Bool, 5=Null, 6=ClosingBracket

Item {
    id: delegateRoot

    required property TreeView treeView
    required property bool isTreeNode
    required property bool expanded
    required property bool hasChildren
    required property int depth
    required property int row
    required property bool current

    // Model roles
    required property string nodeKey
    required property string nodeValue
    required property int nodeType
    required property int childCount
    required property bool showComma

    implicitWidth: rowLayout.implicitWidth + leftPadding
    implicitHeight: Math.max(rowLayout.implicitHeight, 28)

    readonly property int leftPadding: 8 + depth * 20
    readonly property bool _isClosingBracket: delegateRoot.nodeType === 6

    // Syntax roles are resolved by the active theme.
    readonly property color _keyColor:     UTComponentUtil.getPlainUIColor(UIColorToken.Syntax_Key, UIColorState.Normal)
    readonly property color _stringColor:  UTComponentUtil.getPlainUIColor(UIColorToken.Syntax_String, UIColorState.Normal)
    readonly property color _numberColor:  UTComponentUtil.getPlainUIColor(UIColorToken.Syntax_Number, UIColorState.Normal)
    readonly property color _boolColor:    UTComponentUtil.getPlainUIColor(UIColorToken.Syntax_Literal, UIColorState.Normal)
    readonly property color _nullColor:    UTComponentUtil.getPlainUIColor(UIColorToken.Syntax_Literal, UIColorState.Normal)
    readonly property color _bracketColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Text, UIColorState.Normal)
    readonly property color _punctColor:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Text, UIColorState.Normal)
    readonly property color _arrowColor:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal)
    readonly property color _collapsedHint: UTComponentUtil.getPlainUIColor(UIColorToken.Syntax_Comment, UIColorState.Normal)

    Rectangle {
        anchors.fill: parent
        color: delegateRoot.current
               ? UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Selected)
               : (hoverArea.containsMouse
                  ? UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Hovered)
                  : "transparent")
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (!delegateRoot.hasChildren || delegateRoot._isClosingBracket)
                return
            if (delegateRoot.expanded)
                treeView.collapse(delegateRoot.row)
            else
                treeView.expandRecursively(delegateRoot.row, -1)
        }
    }

    Row {
        id: rowLayout
        x: delegateRoot.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        spacing: 2

        // Expand/collapse arrow (only for container nodes, not closing brackets)
        Text {
            id: arrow
            visible: delegateRoot.hasChildren && !delegateRoot._isClosingBracket
            text: delegateRoot.expanded ? "\u25BC" : "\u25B6"
            // glyph size, intentionally not tokenized
            font.pixelSize: 12
            color: delegateRoot._arrowColor
            anchors.verticalCenter: parent.verticalCenter
            width: 16
        }

        // Spacer when no arrow (not needed for closing brackets — they rely on depth indentation)
        Item {
            visible: !delegateRoot.hasChildren && !delegateRoot._isClosingBracket
            width: 16
            height: 1
        }

        // Key label (not for closing brackets)
        Text {
            id: keyLabel
            visible: delegateRoot.nodeKey.length > 0 && !delegateRoot._isClosingBracket
            text: "\"" + delegateRoot.nodeKey + "\""
            font: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)
            color: delegateRoot._keyColor
            anchors.verticalCenter: parent.verticalCenter
        }

        // Colon separator
        Text {
            visible: delegateRoot.nodeKey.length > 0 && !delegateRoot._isClosingBracket
            text: ": "
            font: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)
            color: delegateRoot._punctColor
            anchors.verticalCenter: parent.verticalCenter
        }

        // Value / bracket display
        Text {
            id: valueLabel
            text: _displayText()
            font: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)
            color: _displayColor()
            anchors.verticalCenter: parent.verticalCenter

            function _displayText() {
                switch (delegateRoot.nodeType) {
                case 0: // Object
                    if (delegateRoot.expanded)
                        return "{"
                    return "{...}"
                case 1: // Array
                    if (delegateRoot.expanded)
                        return "["
                    return "[...]"
                case 2: // String
                    return "\"" + delegateRoot.nodeValue + "\""
                case 6: // ClosingBracket
                    return delegateRoot.nodeValue
                default:
                    return delegateRoot.nodeValue
                }
            }

            function _displayColor() {
                switch (delegateRoot.nodeType) {
                case 0: return delegateRoot._bracketColor
                case 1: return delegateRoot._bracketColor
                case 2: return delegateRoot._stringColor
                case 3: return delegateRoot._numberColor
                case 4: return delegateRoot._boolColor
                case 5: return delegateRoot._nullColor
                case 6: return delegateRoot._bracketColor
                default: return delegateRoot._punctColor
                }
            }
        }

        // Trailing comma
        // For expanded containers, comma goes on closing bracket, not on opening line
        Text {
            visible: delegateRoot.showComma
                     && !(delegateRoot.expanded && (delegateRoot.nodeType === 0 || delegateRoot.nodeType === 1))
            text: ","
            font: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)
            color: delegateRoot._punctColor
            anchors.verticalCenter: parent.verticalCenter
        }

        // Collapsed item count hint
        Text {
            visible: !delegateRoot.expanded && (delegateRoot.nodeType === 0 || delegateRoot.nodeType === 1)
            text: " // " + delegateRoot.childCount + (delegateRoot.childCount === 1 ? " item" : " items")
            font: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)
            color: delegateRoot._collapsedHint
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
