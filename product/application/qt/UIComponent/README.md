# UIComponent 开发约定

按“通用机制与渲染骨架、产品视觉与简单组合、复杂组件自身契约”划分职责，不按函数数量、子控件数量或代码行数划分。

## 模块职责

| 位置 | 负责内容 |
| --- | --- |
| UIComponentBase | 可复用的值与索引同步、校验、键盘与焦点、模型适配、生命周期，以及与这些机制耦合的渲染骨架。 |
| UTComponent | 颜色与字体 Token、视觉状态映射、形状与密度、动效选择，以及焦点环、提示等简单装饰和组合。 |
| UTComposite | 可独立使用的复杂 UI 单元，拥有自身的公开值、约束、生命周期和子组件协调。 |
| 宿主页面、业务 Controller / Service | 业务数据来源、权限与业务规则、确认策略、持久化，以及实际业务命令。 |

Base 可以保留 Text、Rectangle、TextInput、ListView、delegate 和可覆盖的中性默认值。UT 可以保留颜色选择函数、派生字体与局部 hover 状态。Composite 可以拥有自身的 QML 辅助函数、Controller、Model 和局部布局样式。

## 依赖方向

组件模块的依赖方向为 `UTComposite → UTComponent → UIComponentBase`；Composite 也可以直接使用 Base 或 Qt 类型。

- Base 不依赖 UT、Composite、产品颜色与字体 Token 或业务模块；可以使用 Qt 和通用底层工具。
- UT 不依赖 Composite 或业务模块。
- Composite 优先组合 UT 控件并遵循统一 Token；组件通过请求与结果表达用户意图，由宿主执行具体业务操作。

## 实现选择

1. 优先复用 Qt 已有的属性和机制，避免自建同义属性或重复的值同步、编辑提交逻辑。
2. 共享绘制结构稳定、只有少量视觉差异时，Base 保留骨架，UT 使用现有属性或少量语义明确的参数配置；产品视觉由 UT 决定。
3. 视觉结构差异较大时，优先使用 background、contentItem、indicator、delegate 等完整槽位，避免不断增加细碎的 Base 样式参数。
4. 每个状态保留一个权威来源。复杂组件自身的值同步与约束留在 Composite，不因为包含函数就下移到 Base。
5. 不要求每个 UT 都有同名 Base，也不要求每个 Composite 再配 BaseComposite。已有薄包装按实际兼容价值处理，不顺带删除。
6. 独立复用需求、异步或平台工作、性能或类型边界可以支持新增辅助模块；文件长或包含几个函数本身不构成拆分理由。

## 当前代码中的职责示例

- [BaseSpinBox](UIComponentBase/BaseSpinBox/BaseSpinBox.qml) 保留输入、格式化、校验与提交骨架；[UTSpinBox](UTComponent/UTSpinBox/UTSpinBox.qml) 决定样式。
- [BaseComboBox](UIComponentBase/BaseComboBox/BaseComboBox.qml) 管理弹层模型、索引与滚动；[UTComboBox](UTComponent/UTComboBox/UTComboBox.qml) 提供文字、图标和提示等视觉组合。
- [UTDateTimeRangePicker](UTComposite/UTDateTimeRangePicker/UTDateTimeRangePicker.qml) 拥有起止值、双端同步与交叉约束，这些函数属于该复杂组件。
- [UTFolderView](UTComposite/UTFolderView/UTFolderView.qml) 拥有目录加载、刷新与选择恢复；打开、复制、删除等请求交给宿主处理。
- [UTMessageDialogController](UTComposite/UTMessageDialog/UTMessageDialogController.h) 拥有按钮模型、默认与取消语义，以及结果只发送一次的控制。

## 色彩约定

具体色值统一放在 `palette.json`，用途放在 `token.json`，浅深主题及交互状态由 `themes/light.json`、`themes/dark.json` 映射。交互强调使用青绿，页面、卡片、输入和弹层使用中性色层级；成功、警告、错误、信息及语法高亮保留各自语义。实填选中背景必须配套选中文字色，温和选中背景可以保留正文或强调文字色。

截图、录屏和视频区域使用跨主题一致的 `Media_*`、`Button_Overlay_*`，透明遮罩由对应 Token 配合 `Qt.alpha()` 合成。二维码黑白、用户标注调色板与 alpha 蒙版属于功能或内容颜色，不随界面强调色替换。主题预览通过只读指定主题取色接口读取同一份 JSON，不切换当前主题、不维护独立预览色表。

Base 的 `selectedTextColor` 等属性是中性渲染槽，默认跟随原 `textColor`；UT 决定配套的选中背景、文字和其他视觉状态。Base 不引用主题 Token。

## 按钮视觉语义

`UTButton.appearance` 使用 `UTButton.Primary / Secondary / Ghost / Destructive`：主操作为青绿色实填，次操作为中性填充与边框，轻操作默认透明，危险操作为红色实填。`UTButton` 保留 Primary 默认值；页面应按操作组显式选择，通常只突出一个主操作。`UTToolButton` 默认 Ghost，沿用相同枚举。

按钮外观与当前焦点、Enter 默认动作、Esc 取消动作分别设置。确认删除使用 Destructive；错误提示中的关闭按钮不因消息图标是 Error 就变为危险操作。消息框的 Controller 提供业务角色，QML 选择对应的 UT 外观，Controller 不指定颜色。

背景、边框、文字及各交互状态均通过主题 Token 配套定义；保留原有 Token 属性以支持明确的局部覆盖。截图、录屏等固定深底工具条使用 `Button_Overlay_Text / Background`，保持浅深主题下的白字和深色交互背景。对话框使用独立 `Dialog_Background`，不改变主窗口背景。焦点圈继续使用独立 `Focus_Outline`。

## 修改与验证

每次调整先说明具体问题、状态归属、公开 API 和默认行为的影响；优先按一个完整职责目标组织小批次改动。
保留现有 UT 默认外观和使用方式，确需变化时明确列出；按改动验证值与模型、绑定、尺寸与命中区、焦点与键盘、RTL 等相关行为。
搬动代码行数、清空 Base 或增加层数不作为改进目标。
