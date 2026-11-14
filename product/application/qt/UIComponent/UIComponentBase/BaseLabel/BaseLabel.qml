import QtQuick
import QtQuick.Controls

Label {
    id: control

    property bool linkEnabled: false
    property bool _linkHovered: false

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    textFormat: linkEnabled ? Text.StyledText : Text.PlainText
    activeFocusOnTab: linkEnabled

    onHoveredLinkChanged: _linkHovered = (hoveredLink !== "")

    Accessible.role: linkEnabled ? Accessible.Link : Accessible.StaticText
    Accessible.name: linkEnabled ? text.replace(/<[^>]*>/g, "") : text

    Keys.onPressed: (event) => {
        if (control.linkEnabled &&
            (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
            const match = text.match(/href=["']([^"']*)["']/)
            if (match) {
                if (!event.isAutoRepeat)
                    control.linkActivated(match[1])
                event.accepted = true
            }
        }
    }
}
