import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import UTComponent
import UIResourceLoader 1.0

/**
 * ScreenshotOverlay - Fullscreen overlay for region selection and annotation
 *
 * Flow:
 * 1. Window opens with transparent background, cursor = crosshair
 * 2. User presses mouse → capture screen, start selection
 * 3. Drag to select region (selected area bright, rest dimmed)
 * 4. Release mouse → show floating toolbar below selection
 * 5. Draw annotations (optional)
 * 6. Save / Cancel
 */
Window {
    id: overlayWindow
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    // Do NOT use Window.FullScreen — that creates a macOS Space visible in Mission Control.
    // Instead, manually cover the entire screen.
    x: Screen.virtualX
    y: Screen.virtualY
    width: Screen.width
    height: Screen.height
    visible: true
    color: "transparent"

    required property var controller

    // Initial screenshot data passed from parent (captured before overlay opens)
    property string initialScreenshot: ""

    // State
    property bool hasScreenshot: initialScreenshot.length > 0
    property string screenshotBase64: initialScreenshot
    property int imgWidth: 0
    property int imgHeight: 0

    // Selection state
    property bool saving: false
    property string saveError: ""
    property bool isSelecting: false
    property bool selectionComplete: false
    property real selStartX: 0
    property real selStartY: 0
    property real selEndX: 0
    property real selEndY: 0

    // Current tool - default to freehand for immediate drawing
    property string currentTool: "freehand"  // none, rectangle, ellipse, arrow, freehand, text
    property string currentColor: "#FF0000" // User annotation content; independent of UI theme.
    property int currentThickness: 3

    // Annotations
    property var annotations: []
    property var currentAnnotation: null

    // Computed selection rect
    property real selX: Math.min(selStartX, selEndX)
    property real selY: Math.min(selStartY, selEndY)
    property real selW: Math.abs(selEndX - selStartX)
    property real selH: Math.abs(selEndY - selStartY)

    // Restore the main app window whenever the overlay closes
    // (Escape, Cancel button, or successful Save).
    onClosing: {
        controller.restoreMainWindow()
    }

    // Close on Escape
    Shortcut {
        sequence: "Escape"
        onActivated: overlayWindow.close()
    }

    // Main content
    Item {
        anchors.fill: parent

        // Screenshot background (only visible after capture)
        Image {
            id: screenshotBg
            anchors.fill: parent
            source: hasScreenshot ? "data:image/png;base64," + screenshotBase64 : ""
            fillMode: Image.PreserveAspectFit
            visible: hasScreenshot
            cache: false
        }

        // Dark overlay (covers everything except selection)
        Canvas {
            id: overlayCanvas
            anchors.fill: parent
            visible: hasScreenshot
            readonly property color selectionColor: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Selection, UIColorState.Normal)
            readonly property color scrimColor: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Scrim, UIColorState.Normal)
            onSelectionColorChanged: requestPaint()
            onScrimColorChanged: requestPaint()

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                // Draw semi-transparent dark overlay
                ctx.fillStyle = Qt.alpha(overlayCanvas.scrimColor, 0.5)
                ctx.fillRect(0, 0, width, height)

                // Cut out the selection area (make it bright)
                if (isSelecting || selectionComplete) {
                    ctx.globalCompositeOperation = "destination-out"
                    ctx.fillStyle = overlayCanvas.scrimColor
                    ctx.fillRect(selX, selY, selW, selH)
                    ctx.globalCompositeOperation = "source-over"

                    // Draw selection border
                    ctx.strokeStyle = overlayCanvas.selectionColor
                    ctx.lineWidth = 2
                    ctx.setLineDash([5, 5])
                    ctx.strokeRect(selX, selY, selW, selH)
                    ctx.setLineDash([])

                    // Draw size indicator
                    if (selW > 50 && selH > 20) {
                        var sizeText = Math.round(selW) + " × " + Math.round(selH)
                        ctx.font = "12px Arial"
                        ctx.fillStyle = overlayCanvas.selectionColor
                        ctx.fillText(sizeText, selX + 5, selY + 15)
                    }
                }
            }
        }

        // Annotation canvas (for drawing on selection)
        Canvas {
            id: annotationCanvas
            x: selX
            y: selY
            width: selW > 0 ? selW : 1
            height: selH > 0 ? selH : 1
            visible: selectionComplete

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                // Draw all annotations
                for (var i = 0; i < annotations.length; i++) {
                    drawAnnotation(ctx, annotations[i])
                }

                // Draw current annotation being drawn
                if (currentAnnotation) {
                    drawAnnotation(ctx, currentAnnotation)
                }
            }

            function drawAnnotation(ctx, ann) {
                ctx.strokeStyle = ann.color || "#FF0000"
                ctx.fillStyle = ann.color || "#FF0000"
                ctx.lineWidth = ann.thickness || 3

                if (ann.type === "rectangle") {
                    ctx.strokeRect(ann.x, ann.y, ann.w, ann.h)
                } else if (ann.type === "ellipse") {
                    ctx.beginPath()
                    ctx.ellipse(ann.x + ann.w/2, ann.y + ann.h/2, ann.w/2, ann.h/2, 0, 0, 2 * Math.PI)
                    ctx.stroke()
                } else if (ann.type === "arrow") {
                    // Line
                    ctx.beginPath()
                    ctx.moveTo(ann.x1, ann.y1)
                    ctx.lineTo(ann.x2, ann.y2)
                    ctx.stroke()
                    // Arrowhead
                    var angle = Math.atan2(ann.y2 - ann.y1, ann.x2 - ann.x1)
                    var arrowSize = 10 + ann.thickness
                    ctx.beginPath()
                    ctx.moveTo(ann.x2, ann.y2)
                    ctx.lineTo(ann.x2 - arrowSize * Math.cos(angle - Math.PI/6),
                              ann.y2 - arrowSize * Math.sin(angle - Math.PI/6))
                    ctx.lineTo(ann.x2 - arrowSize * Math.cos(angle + Math.PI/6),
                              ann.y2 - arrowSize * Math.sin(angle + Math.PI/6))
                    ctx.closePath()
                    ctx.fill()
                } else if (ann.type === "freehand" && ann.points && ann.points.length > 1) {
                    ctx.beginPath()
                    ctx.moveTo(ann.points[0].x, ann.points[0].y)
                    for (var i = 1; i < ann.points.length; i++) {
                        ctx.lineTo(ann.points[i].x, ann.points[i].y)
                    }
                    ctx.stroke()
                } else if (ann.type === "text") {
                    ctx.font = (ann.fontSize || 16) + "px Arial"
                    ctx.fillText(ann.text, ann.x, ann.y)
                }
            }
        }

        // Mouse area for selection and annotation
        MouseArea {
            id: mainMouseArea
            anchors.fill: parent
            cursorShape: hasScreenshot ? (selectionComplete ? Qt.ArrowCursor : Qt.CrossCursor) : Qt.CrossCursor
            hoverEnabled: true

            property real drawStartX: 0
            property real drawStartY: 0
            property var freehandPoints: []

            onPressed: function(mouse) {
                if (!selectionComplete) {
                    // Continue/restart selection
                    isSelecting = true
                    selStartX = mouse.x
                    selStartY = mouse.y
                    selEndX = mouse.x
                    selEndY = mouse.y
                } else if (currentTool !== "none") {
                    // Drawing annotation
                    var localX = mouse.x - selX
                    var localY = mouse.y - selY

                    // Check if inside selection
                    if (localX >= 0 && localX <= selW && localY >= 0 && localY <= selH) {
                        drawStartX = localX
                        drawStartY = localY
                        freehandPoints = [{x: localX, y: localY}]

                        if (currentTool === "text") {
                            textInputDialog.inputX = localX
                            textInputDialog.inputY = localY
                            textInputDialog.open()
                        } else {
                            currentAnnotation = {
                                type: currentTool,
                                color: currentColor,
                                thickness: currentThickness,
                                x: localX, y: localY, w: 0, h: 0,
                                x1: localX, y1: localY, x2: localX, y2: localY,
                                points: freehandPoints
                            }
                        }
                    }
                }
                overlayCanvas.requestPaint()
            }

            onPositionChanged: function(mouse) {
                if (isSelecting) {
                    selEndX = mouse.x
                    selEndY = mouse.y
                    overlayCanvas.requestPaint()
                } else if (currentAnnotation && currentTool !== "none" && currentTool !== "text") {
                    var localX = mouse.x - selX
                    var localY = mouse.y - selY

                    if (currentTool === "freehand") {
                        freehandPoints.push({x: localX, y: localY})
                        currentAnnotation.points = freehandPoints
                    } else {
                        currentAnnotation.w = localX - drawStartX
                        currentAnnotation.h = localY - drawStartY
                        currentAnnotation.x2 = localX
                        currentAnnotation.y2 = localY
                    }
                    annotationCanvas.requestPaint()
                }
            }

            onReleased: function(mouse) {
                if (isSelecting) {
                    isSelecting = false
                    selEndX = mouse.x
                    selEndY = mouse.y

                    // Minimum selection size
                    if (selW > 10 && selH > 10) {
                        selectionComplete = true
                    }
                    overlayCanvas.requestPaint()
                } else if (currentAnnotation && currentTool !== "none" && currentTool !== "text") {
                    // Finalize annotation
                    var localX = mouse.x - selX
                    var localY = mouse.y - selY

                    if (currentTool === "rectangle" || currentTool === "ellipse") {
                        currentAnnotation.x = Math.min(drawStartX, localX)
                        currentAnnotation.y = Math.min(drawStartY, localY)
                        currentAnnotation.w = Math.abs(localX - drawStartX)
                        currentAnnotation.h = Math.abs(localY - drawStartY)
                    }

                    // Add to annotations if significant
                    if (currentTool === "freehand" && freehandPoints.length > 2) {
                        annotations.push(currentAnnotation)
                    } else if (currentAnnotation.w > 5 || currentAnnotation.h > 5 ||
                               Math.abs(currentAnnotation.x2 - currentAnnotation.x1) > 5 ||
                               Math.abs(currentAnnotation.y2 - currentAnnotation.y1) > 5) {
                        annotations.push(currentAnnotation)
                    }

                    currentAnnotation = null
                    freehandPoints = []
                    annotationCanvas.requestPaint()
                }
            }
        }

        // Floating toolbar (appears below selection)
        Rectangle {
            id: toolbar
            visible: selectionComplete
            x: Math.max(10, Math.min(selX, parent.width - width - 10))
            y: selY + selH + 10 < parent.height - 60 ? selY + selH + 10 : selY - height - 10
            width: toolbarRow.width + 20
            height: 44
            radius: 8
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Surface, UIColorState.Normal)
            border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Border, UIColorState.Normal)
            border.width: 1

            RowLayout {
                id: toolbarRow
                anchors.centerIn: parent
                spacing: 4

                // Tool buttons with explicit checked styling
                UTToolButton {
                    appearance: UTButton.Ghost
                    fontColorEnum: UIColorToken.Button_Overlay_Text
                    text: "🔲"
                    checked: currentTool === "rectangle"
                    onClicked: currentTool = (currentTool === "rectangle" ? "none" : "rectangle")
                    toolTipText: qsTr("Rectangle")
                    Accessible.name: toolTipText
                    background: Rectangle {
                        color: parent.checked ? UTComponentUtil.getPlainUIColor(UIColorToken.Badge_Accent_Background, UIColorState.Normal) : (parent.hovered ? UTComponentUtil.getPlainUIColor(UIColorToken.Button_Overlay_Background, UIColorState.Hovered) : "transparent")
                        radius: 4
                    }
                }

                UTToolButton {
                    appearance: UTButton.Ghost
                    fontColorEnum: UIColorToken.Button_Overlay_Text
                    text: "⭕"
                    checked: currentTool === "ellipse"
                    onClicked: currentTool = (currentTool === "ellipse" ? "none" : "ellipse")
                    toolTipText: qsTr("Ellipse")
                    Accessible.name: toolTipText
                    background: Rectangle {
                        color: parent.checked ? UTComponentUtil.getPlainUIColor(UIColorToken.Badge_Accent_Background, UIColorState.Normal) : (parent.hovered ? UTComponentUtil.getPlainUIColor(UIColorToken.Button_Overlay_Background, UIColorState.Hovered) : "transparent")
                        radius: 4
                    }
                }

                UTToolButton {
                    appearance: UTButton.Ghost
                    fontColorEnum: UIColorToken.Button_Overlay_Text
                    text: "➡️"
                    checked: currentTool === "arrow"
                    onClicked: currentTool = (currentTool === "arrow" ? "none" : "arrow")
                    toolTipText: qsTr("Arrow")
                    Accessible.name: toolTipText
                    background: Rectangle {
                        color: parent.checked ? UTComponentUtil.getPlainUIColor(UIColorToken.Badge_Accent_Background, UIColorState.Normal) : (parent.hovered ? UTComponentUtil.getPlainUIColor(UIColorToken.Button_Overlay_Background, UIColorState.Hovered) : "transparent")
                        radius: 4
                    }
                }

                UTToolButton {
                    appearance: UTButton.Ghost
                    fontColorEnum: UIColorToken.Button_Overlay_Text
                    text: "✏️"
                    checked: currentTool === "freehand"
                    onClicked: currentTool = (currentTool === "freehand" ? "none" : "freehand")
                    toolTipText: qsTr("Freehand")
                    Accessible.name: toolTipText
                    background: Rectangle {
                        color: parent.checked ? UTComponentUtil.getPlainUIColor(UIColorToken.Badge_Accent_Background, UIColorState.Normal) : (parent.hovered ? UTComponentUtil.getPlainUIColor(UIColorToken.Button_Overlay_Background, UIColorState.Hovered) : "transparent")
                        radius: 4
                    }
                }

                UTToolButton {
                    appearance: UTButton.Ghost
                    fontColorEnum: UIColorToken.Button_Overlay_Text
                    text: "T"
                    checked: currentTool === "text"
                    onClicked: currentTool = (currentTool === "text" ? "none" : "text")
                    toolTipText: qsTr("Text")
                    Accessible.name: toolTipText
                    background: Rectangle {
                        color: parent.checked ? UTComponentUtil.getPlainUIColor(UIColorToken.Badge_Accent_Background, UIColorState.Normal) : (parent.hovered ? UTComponentUtil.getPlainUIColor(UIColorToken.Button_Overlay_Background, UIColorState.Hovered) : "transparent")
                        radius: 4
                    }
                }

                // Separator
                Rectangle { width: 1; height: 24; color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Border, UIColorState.Normal) }

                // Color buttons
                Repeater {
                    model: ["#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#FFFFFF", "#000000"]
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 10
                        color: modelData
                        border.width: currentColor === modelData ? 2 : 1
                        border.color: currentColor === modelData ? UTComponentUtil.getPlainUIColor(UIColorToken.Media_Selection, UIColorState.Normal) : UTComponentUtil.getPlainUIColor(UIColorToken.Media_Secondary_Text, UIColorState.Normal)

                        MouseArea {
                            anchors.fill: parent
                            onClicked: currentColor = modelData
                        }
                    }
                }

                // Separator
                Rectangle { width: 1; height: 24; color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Border, UIColorState.Normal) }

                // Undo
                ToolButton {
                    id: undoToolButton
                    text: "↩️"
                    enabled: annotations.length > 0
                    palette.button: toolbar.color
                    palette.mid: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Border, UIColorState.Normal)
                    palette.buttonText: UTComponentUtil.getItemUIColor(undoToolButton, UIColorToken.Button_Overlay_Text)
                    palette.highlight: palette.buttonText
                    onClicked: {
                        if (annotations.length > 0) {
                            annotations.pop()
                            annotations = annotations  // Trigger update
                            annotationCanvas.requestPaint()
                        }
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Undo")
                }

                // Separator
                Rectangle { width: 1; height: 24; color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Border, UIColorState.Normal) }

                // Cancel button
                UTToolButton {
                    appearance: UTButton.Ghost
                    fontColorEnum: UIColorToken.Button_Overlay_Text
                    text: "✕"
                    onClicked: overlayWindow.close()
                    toolTipText: qsTr("Cancel")
                    Accessible.name: toolTipText

                    background: Rectangle {
                        radius: 4
                        color: (parent.pressed || parent.hovered) ? UTComponentUtil.getItemUIColor(parent, UIColorToken.Button_Destructive_Background) : "transparent"
                    }
                }

                // Save button
                ToolButton {
                    id: saveToolButton
                    text: "✓"
                    enabled: !saving
                    palette.buttonText: UTComponentUtil.getItemUIColor(saveToolButton,
                        saveToolButton.enabled && (saveToolButton.pressed || saveToolButton.hovered)
                        ? UIColorToken.Button_Primary_Text : UIColorToken.Button_Overlay_Text)
                    palette.highlight: palette.buttonText
                    onClicked: saveScreenshot()
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Save")

                    background: Rectangle {
                        radius: 4
                        color: saveToolButton.enabled && (saveToolButton.pressed || saveToolButton.hovered)
                               ? UTComponentUtil.getItemUIColor(saveToolButton, UIColorToken.Button_Primary_Background)
                               : "transparent"
                    }
                }
            }
        }

        // Keep export errors visible inside this fullscreen editing window.
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 20
            width: Math.min(520, parent.width - 40)
            height: saveErrorText.implicitHeight + 24
            radius: 8
            color: Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Media_Error_Background, UIColorState.Normal), 0.9)
            border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Recording_Pulse, UIColorState.Normal)
            visible: saveError.length > 0
            z: 10

            Text {
                id: saveErrorText
                anchors.fill: parent
                anchors.margins: 12
                text: saveError
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Text, UIColorState.Normal)
                wrapMode: Text.Wrap
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Large)
            }
        }

        // Text input dialog
        Dialog {
            id: textInputDialog
            title: qsTr("Add Text")
            modal: true
            anchors.centerIn: parent
            standardButtons: Dialog.Ok | Dialog.Cancel

            property real inputX: 0
            property real inputY: 0

            contentItem: ColumnLayout {
                spacing: 12

                UTTextField {
                    id: textField
                    Layout.fillWidth: true
                    Layout.preferredWidth: 200
                    placeholderText: qsTr("Enter text...")
                }

                UTSpinBox {
                    id: fontSizeBox
                    from: 8
                    to: 72
                    value: 16
                }
            }

            onAccepted: {
                if (textField.text.length > 0) {
                    annotations.push({
                        type: "text",
                        x: inputX,
                        y: inputY,
                        text: textField.text,
                        color: currentColor,
                        fontSize: fontSizeBox.value
                    })
                    annotations = annotations
                    annotationCanvas.requestPaint()
                }
                textField.text = ""
            }
        }

        // Instructions overlay (before capture)
        Rectangle {
            anchors.centerIn: parent
            width: instructionText.width + 40
            height: instructionText.height + 20
            radius: 8
            color: Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Media_Scrim, UIColorState.Normal), 0.8)
            visible: !hasScreenshot

            Text {
                id: instructionText
                anchors.centerIn: parent
                text: qsTr("Click and drag to select region\nPress ESC to cancel")
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Media_Text, UIColorState.Normal)
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Large)
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    function saveScreenshot() {
        if (saving)
            return

        saving = true
        saveError = ""
        try {
            // Calculate image offset due to PreserveAspectFit centering.
            var offsetX = (overlayWindow.width - screenshotBg.paintedWidth) / 2
            var offsetY = (overlayWindow.height - screenshotBg.paintedHeight) / 2
            var imageX = selX - offsetX
            var imageY = selY - offsetY

            var result = controller.saveRegionScreenshot(
                imageX, imageY, selW, selH,
                screenshotBg.paintedWidth, screenshotBg.paintedHeight,
                annotations
            )
            if (result.success) {
                overlayWindow.close()
            } else {
                saveError = result.error || qsTr("Could not save the screenshot. Please try again.")
            }
        } catch (error) {
            saveError = qsTr("Could not save the screenshot. Please try again.")
            console.error("Failed to save screenshot:", error)
        } finally {
            saving = false
        }
    }
}
