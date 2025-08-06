import UIComponentBase 1.0
import UIElementData 1.0
import UTComponent 1.0

BaseFocusItem {
    id: control
    focusColor: UTComponentUtil.getPlainUIColor(UIElementData.UIColorEnum_Item_Focus_Border, UIElementData.UIColorState_Normal);
    focusRadius: Math.min(control.height, control.width)/2
}