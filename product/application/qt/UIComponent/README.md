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

## 修改与验证

每次调整先说明具体问题、状态归属、公开 API 和默认行为的影响；优先按一个完整职责目标组织小批次改动。
保留现有 UT 默认外观和使用方式，确需变化时明确列出；按改动验证值与模型、绑定、尺寸与命中区、焦点与键盘、RTL 等相关行为。
搬动代码行数、清空 Base 或增加层数不作为改进目标。
