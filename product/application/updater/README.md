# Updater

独立的升级程序，接收主应用已经解包并整理好的 staging 目录，在旧进程退出后替换安装目录。

## 工作流程

1. 主应用下载、校验并解包更新，检查 staging 中的主程序入口。
2. 主应用把 staging 内的 updater 复制到应用临时目录，写入 `upgrade_in_progress`，启动 updater。
3. 启动接口返回成功后，主应用请求退出。
4. updater 再次检查 staging 主程序入口，修复 staging 权限，并等待旧进程退出（默认最多 30 秒）。
5. 确认当前安装入口完整后，清理上一轮备份，把当前安装目录重命名为 `<target>.bak`。
6. 把 staging 重命名为 target；失败时，仅在 target 仍为空缺的情况下尝试把本次备份改回 target。
7. 检查替换后的入口并提交新版启动请求。
8. 启动请求成功后清理 marker，保留一份 `<target>.bak`，下一次升级通过准备检查后再替换这份备份。

等待超时、无法查询旧进程、旧备份清理失败时停止替换。替换后的入口检查或启动请求失败时，保留新 target、旧 backup 和 marker，并返回非零状态。

启动请求成功不代表新版已完成初始化。新版之后启动失败时，保留的 `.bak` 可用于人工恢复；本实现尚无新版健康确认或完整的中断恢复协议。

## 命令行参数

```text
updater --staging <预解包目录> --target <安装目录> --pid <旧进程PID> [--restart]
```

| 参数 | 必需 | 说明 |
|---|---|---|
| `--staging` | 是 | 已整理为安装根布局的更新目录 |
| `--target` | 是 | 当前安装目录；macOS 为 `.app` bundle |
| `--pid` | 是 | 正数旧进程 PID；省略或非正数会停止升级 |
| `--restart` | 否 | 提交新版启动请求（配置默认开启） |

## 平台约定

| 平台 | 主程序入口 | 启动方式 |
|---|---|---|
| Windows | `<root>/bin/mainEntry.exe` | `CreateProcessW` |
| macOS | `<root>/Contents/MacOS/mainEntry`，并要求 `Contents/Info.plist` | `posix_spawn` 调用 `/usr/bin/open`，检查 open 的退出结果 |
| Linux | `<root>/bin/mainEntry` | `posix_spawn`，检查启动返回码 |

staging、target 不得是文件系统根目录，staging 与 target/backup 不得相同或互相包含；比较前先解析现有路径并规范化。

入口必须为非空普通文件。Unix 在 staging 中对原有权限修复规则覆盖的文件添加执行权限；Windows 无此步骤。重命名最多尝试 5 次，间隔依次为 1、2、3、4 秒。

## Linux 包布局约定

升级 manifest 继续选择 `.tar.gz`（TGZ）。TGZ 是便携应用目录：CPack 顶层产品目录内直接包含 `bin/`、`lib/`、`share/` 等应用内容；UpgradeService 提升一层产品目录后，staging 根目录下应直接有 `bin/mainEntry` 和 `bin/updater`。

DEB/RPM 保留系统安装布局：应用内容放在 `/opt/<package>/`，desktop/icon 集成放在 `/usr/share/`，命令 wrapper 放在 `/usr/bin/`。这些系统集成操作只在 DEB/RPM 的 pre-build hook 中执行，TGZ 不执行。

此处修正的是 TGZ 内容与目录替换 updater 的布局契约，不包含系统安装提权或包管理器事务。用 DEB/RPM 安装的系统程序仍不能因此获得完整的包管理器自更新支持。

## 日志与失败记录

日志文件是运行中 updater 所在目录下的 `updater.log`，以追加方式写入。正常由主应用启动时，该目录为 `<应用临时目录>/template-factory-upgrade`；应用日志中的 updater 启动路径可用于定位。

失败时 updater 保留 `upgrade_in_progress`。当前主应用下次启动会记录中断警告并清理 marker/staging，不会自动根据 `.bak` 恢复安装。备份目录位于安装目录旁。

## 返回值

升级流程返回 `UpgradeExitCode`，仅在 `main()` 返回给操作系统时转换为整数；数值保持稳定。

| 枚举 | 数值 | 含义 |
|---|---|---|
| `Success` | 0 | 文件替换完成；若配置重启，启动请求已被接受 |
| `InvalidInput` | 1 | 必要参数缺失、PID 非正数、路径无效或 staging 入口无效 |
| `BackupPreparationFailed` | 2 | 当前安装不完整，或工作目录、旧备份清理、创建备份失败 |
| `ReplacementFailed` | 3 | staging 替换失败；日志记录本次 rollback 结果 |
| `InstalledLayoutInvalid` | 4 | 替换后的主程序入口无效 |
| `ParentExitUnconfirmed` | 5 | 等待旧进程退出超时或查询失败 |
| `ApplicationLaunchFailed` | 6 | 新版启动请求失败 |
| `UnexpectedError` | 7 | 其他升级异常 |

## 人工恢复

先确认新版与 updater 已经退出，再检查 target 和 `<target>.bak`。如需恢复旧版，先将现有 target 移到其他位置，再把 `.bak` 改回 target；保留失败日志供排查。

## 源码结构

```text
updater/
├── main.cpp
├── config/
│   ├── UpdaterConfig.h
│   └── UpdaterConfig.cpp
├── core/
│   ├── UpdaterLog.h
│   ├── UpgradeExecutor.h
│   ├── UpgradeExecutor.cpp
│   ├── UpgradeValidation.h
│   └── UpgradeValidation.cpp
└── platform/
    ├── PlatformUtils.h
    ├── PlatformUtils_win.cpp
    └── PlatformUtils_unix.cpp
```

## 职责边界

- `UpgradeExecutor`：决定阶段顺序、退出码、备份清理时机和回滚条件。保留自由函数入口 `performUpgrade()`。
- `UpgradeValidation`：返回规范化的 `UpgradePaths`，检查路径关系和平台主入口；不等待进程、不修改文件、不启动应用。
- `PlatformUtils`：实现进程等待、主入口路径、程序启动和权限处理等平台操作。

`UpgradePaths` 是仅含 staging、target、backup 的数据结构，没有阶段标记或资源所有权。路径输入无效时 `resolveUpgradePaths()` 返回 `nullopt`；文件系统查询异常交给 Executor 返回 7。入口检查返回 `bool`，由 Executor 的调用阶段决定返回 `InvalidInput`、`BackupPreparationFailed` 或 `InstalledLayoutInvalid`。重命名重试仍为 Executor 内部函数。
