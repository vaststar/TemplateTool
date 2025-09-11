import UIComponentBase 1.0
import UIResourceLoader 1.0
import UTComponent 1.0

BaseFocusItem {
    id: control
    focusColor: UTComponentUtil.getPlainUIColor(UIColorToken.FocusOutline, UIColorState.Normal);
    focusRadius: Math.min(control.height, control.width)/2
}