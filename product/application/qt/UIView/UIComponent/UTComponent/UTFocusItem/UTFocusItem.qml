import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UIElementData 1.0

BaseFocusItem {
    id: control
    focusColor: UTComponentUtil.getPlainColor(UIElementData.UIColorEnum_Item_Focus_Border, UIElementData.UIColorState_Normal);
}