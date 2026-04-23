# Updater

独立的升级程序，由主应用在检测到新版本后启动。主应用退出 → updater 替换文件 → 重启新版本。

## 工作流程

1. 主应用下载 ZIP 包到临时目录，记下包路径、安装目录、自身 PID
2. 主应用启动 updater 并退出自身
3. updater 等待主应用进程退出（最多 30s）
4. 将当前安装目录重命名为 `.bak` 作为备份
5. 解压 ZIP 覆盖安装目录
6. 修复权限（仅 macOS/Linux）
7. 启动新版本
8. 清理备份和 ZIP 包

失败时会自动回滚（将 `.bak` 重命名回去）。

## 命令行参数

```
updater --package <zip路径> --target <安装目录> --pid <主应用PID> [--restart]
```

| 参数 | 必需 | 说明 |
|---|---|---|
| `--package` | 是 | 下载的 ZIP 包路径 |
| `--target` | 是 | 应用安装目录 |
| `--pid` | 否 | 主应用 PID，updater 会等它退出后再操作 |
| `--restart` | 否 | 升级完成后自动启动新版本（默认开启） |

## 平台差异

### Windows
- 解压工具：PowerShell `Expand-Archive`
- 启动方式：`CreateProcessW` 启动 `<target>/bin/mainEntry.exe`
- 权限修复：无需处理
- 特殊处理：备份重命名时会重试最多 5 次（杀软/索引器可能短暂锁定文件）

### macOS
- 解压工具：系统 `unzip`
- `--target` 指向 `.app` bundle（如 `/Applications/mainEntry.app`）
- 解压到临时目录，找到 `.app` 后移动到 target 位置
- 启动方式：`open "<target>.app"`
- 权限修复：对无扩展名、`.sh`、`.so`、`.dylib` 文件添加可执行权限

### Linux
- 解压工具：系统 `unzip`
- 启动方式：`fork + execl` 启动 `<target>/bin/mainEntry`
- 权限修复：同 macOS

## 日志

日志同时输出到 stdout/stderr 和文件：

| 平台 | 日志路径 |
|---|---|
| Windows | `%TEMP%\template-factory-upgrade\updater.log` |
| macOS / Linux | `/tmp/template-factory-upgrade/updater.log` |

日志以 `append` 模式写入，多次升级会累积在同一个文件中。

## 错误码

| 返回值 | 含义 |
|---|---|
| 0 | 成功 |
| 1 | 参数错误或包文件不存在 |
| 2 | 备份失败（文件被占用） |
| 3 | 解压失败 |
| 4 | 解压后目标目录为空 |

## 排查问题

1. 先看日志文件（路径见上方），搜索 `ERROR` 或 `Warning`
2. Windows 上升级失败最常见的原因是文件被锁定（返回码 2）—— 确认主应用和相关进程已退出
3. macOS/Linux 上如果解压失败，确认系统装了 `unzip`
4. 如果回滚也失败了，手动将 `<target>.bak` 重命名回 `<target>` 即可恢复

## 源码结构

```
updater/
├── main.cpp                      入口
├── config/
│   ├── UpdaterConfig.h           命令行参数结构体
│   └── UpdaterConfig.cpp         参数解析
├── core/
│   ├── UpdaterLog.h              日志宏（header-only）
│   ├── UpgradeExecutor.h         升级流程声明
│   └── UpgradeExecutor.cpp       升级流程实现
└── platform/
    ├── PlatformUtils.h           平台接口
    ├── PlatformUtils_win.cpp     Windows 实现
    └── PlatformUtils_unix.cpp    macOS/Linux 实现
```
