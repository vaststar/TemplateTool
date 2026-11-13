import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseQRCode {
    id: control
    property string invalidText: qsTr("Invalid QR Code")

    invalidIndicator: UTText {
        text: control.invalidText
        fontEnum: UIFontToken.Caption_Text
        colorEnum: UIColorToken.Content_Secondary_Text
    }
    
    // Default styling for UT components
    displaySize: 200
    errorLevel: UTQRCodeLevel.Medium
    border: 4
    darkColor: "#000000"
    lightColor: "#FFFFFF"
}
