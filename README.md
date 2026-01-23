# TemplateTool 构建指南

本项目使用 **CMake Presets** 实现跨平台统一构建，支持 Windows、macOS 和 Linux。

## 📋 目录

- [前置要求](#前置要求)
- [快速开始](#快速开始)
- [构建脚本说明](#构建脚本说明)
- [可用的构建配置](#可用的构建配置)
- [常见使用场景](#常见使用场景)
- [故障排除](#故障排除)
- [高级用法](#高级用法)

---

## 前置要求

### 所有平台

- **CMake 3.23+** （必需）
  - [下载地址](https://cmake.org/download/)
  - 验证：`cmake --version`

- **Qt 6.5+** （必需）
  - [官方安装器](https://www.qt.io/download-qt-installer)
  - 或使用 aqtinstall 命令行安装：
    ```bash
    pip install aqtinstall
    aqt install-qt mac desktop 6.8.3 clang_64 -O ~/Qt
    ```

### Windows

选择以下编译器之一：

- **Visual Studio 2022** (推荐)
  - 包含 MSVC 编译器
  - 自动检测，无需额外配置

- **MinGW-w64** + Ninja
  ```bash
  # 使用 MSYS2 安装
  pacman -S mingw-w64-x86_64-gcc
  pacman -S mingw-w64-x86_64-ninja
  pacman -S mingw-w64-x86_64-qt6
  ```

### macOS

```bash
# 使用 Homebrew 安装基础工具
brew install cmake

# Qt 建议使用官方安装器或 aqtinstall
pip install aqtinstall
aqt install-qt mac desktop 6.8.3 clang_64 -O ~/Qt
```

### Linux

```bash
# Ubuntu/Debian
sudo apt install cmake ninja-build gcc g++ qt6-base-dev

# Fedora
sudo dnf install cmake ninja-build gcc-c++ qt6-qtbase-devel
```

---

## 快速开始

### Windows

```batch
# 使用 MSVC 编译 Release 版本（推荐）
build.bat windows-msvc-release

# 使用 MinGW 编译
build.bat windows-mingw-release
```

### macOS

```bash
# 编译 Release 版本
./build.sh macos-release

# 编译 Debug 版本
./build.sh macos-debug
```

### Linux

```bash
# 编译 Release 版本
./build.sh linux-release
```

---

## 构建脚本说明

项目提供了两个统一的构建脚本：

| 脚本 | 平台 | 说明 |
|------|------|------|
| `build.bat` | Windows | Windows 构建脚本 |
| `build.sh` | macOS/Linux | Unix 构建脚本 |

### 脚本使用方式

```bash
# 基本语法
./build.sh [preset] [action]

# 或 Windows
build.bat [preset] [action]
```

### 可用的操作（action）

| 操作 | 说明 | 示例 |
|------|------|------|
| **all** *(默认)* | 配置 + 编译 | `./build.sh macos-release` |
| **configure** | 只配置项目，生成构建文件 | `./build.sh macos-release configure` |
| **build** | 只编译（需先配置） | `./build.sh macos-release build` |
| **clean** | 清理编译产物 | `./build.sh macos-release clean` |
| **install** | 编译 + 安装到 install 目录 | `./build.sh macos-release install` |
| **package** | 编译 + 打包成分发包 | `./build.sh macos-release package` |

---

## 可用的构建配置

### Windows 配置

| Preset | 编译器 | 构建类型 | 说明 |
|--------|--------|----------|------|
| `windows-msvc-debug` | MSVC | Debug | 用于调试，包含调试符号 |
| `windows-msvc-release` | MSVC | Release | 优化版本，用于发布 |
| `windows-mingw-debug` | MinGW | Debug | MinGW 调试版本 |
| `windows-mingw-release` | MinGW | Release | MinGW 发布版本 |

### macOS 配置

| Preset | 编译器 | 构建类型 | 说明 |
|--------|--------|----------|------|
| `macos-debug` | Clang | Debug | 用于调试 |
| `macos-release` | Clang | Release | 用于发布 |

### Linux 配置

| Preset | 编译器 | 构建类型 | 说明 |
|--------|--------|----------|------|
| `linux-debug` | GCC | Debug | 用于调试 |
| `linux-release` | GCC | Release | 用于发布 |

---

## 常见使用场景

### 场景 1：开发调试

```bash
# macOS - 编译 Debug 版本
./build.sh macos-debug

# 运行程序
./build/macos-debug/bin/mainEntry
```

### 场景 2：发布版本

```bash
# 编译 Release 版本
./build.sh macos-release

# 打包成分发包
./build.sh macos-release package

# 打包文件位置
# build/macos-release/mainEntry-0.0.0-Darwin.tar.gz
# build/macos-release/mainEntry-0.0.0-Darwin.dmg
```

### 场景 3：清理后重新编译

```bash
# 清理编译产物
./build.sh macos-release clean

# 或者删除整个构建目录
rm -rf build/macos-release

# 重新编译
./build.sh macos-release
```

### 场景 4：只配置不编译（用于 IDE）

```bash
# 生成 compile_commands.json 供 IDE 使用
./build.sh macos-release configure

# 在 VS Code 或 CLion 中打开项目
# IDE 会自动识别 CMakePresets.json
```

### 场景 5：配置 Qt 路径（首次构建必须）

本项目使用 `CMakeUserPresets.json` 管理本地 Qt 路径：

```bash
# 复制模板文件
cp CMakeUserPresets.json.example CMakeUserPresets.json

# 编辑 CMakeUserPresets.json，修改 CMAKE_PREFIX_PATH 为你的 Qt 安装路径
```

示例配置：
```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "macos-release",
      "inherits": "macos-release",
      "cacheVariables": {
        "CMAKE_PREFIX_PATH": "$env{HOME}/Qt/6.8.3/macos"
      }
    }
  ]
}
```

常见 Qt 路径：
| 平台 | 安装方式 | 路径示例 |
|------|----------|----------|
| macOS | 官方安装器/aqtinstall | `~/Qt/6.8.3/macos` |
| macOS | Homebrew | `/opt/homebrew/opt/qt@6`（不推荐） |
| Windows | 官方安装器 | `C:/Qt/6.8.3/msvc2022_64` |
| Linux | 官方安装器/aqtinstall | `~/Qt/6.8.3/gcc_64` |

---

## 故障排除

### 问题 1：CMake 找不到

**错误信息**:
```
[ERROR] System CMake not found.
```

**解决方法**:
```bash
# 安装 CMake
# Windows: https://cmake.org/download/
# macOS: brew install cmake
# Linux: sudo apt install cmake

# 验证安装
cmake --version
```

### 问题 2：找不到 Qt

**错误信息**:
```
CMake Error: Qt6 could not be found.
```

**解决方法**:
```bash
# 1. 确保已安装 Qt
pip install aqtinstall
aqt install-qt mac desktop 6.8.3 clang_64 -O ~/Qt

# 2. 配置 CMakeUserPresets.json
cp CMakeUserPresets.json.example CMakeUserPresets.json
# 编辑文件，设置正确的 Qt 路径

# 3. 重新运行
./build.sh macos-release
```

### 问题 3：Visual Studio 找不到

**错误信息**:
```
[ERROR] Visual Studio generator detection failed
```

**解决方法**:
- 安装 Visual Studio 2022（包含 C++ 工作负载）
- 或使用 MinGW：`build.bat windows-mingw-release`

### 问题 4：Ninja 找不到（macOS/Linux）

**错误信息**:
```
CMake Error: CMake was unable to find a build program corresponding to "Ninja".
```

**解决方法**:
```bash
# macOS
brew install ninja

# Linux
sudo apt install ninja-build
```

### 问题 5：权限不足（macOS/Linux）

**错误信息**:
```
Permission denied
```

**解决方法**:
```bash
# 给脚本添加执行权限
chmod +x build.sh

# 然后运行
./build.sh macos-release
```

### 问题 6：编译器版本不匹配（Windows MinGW）

**错误信息**:
```
incompatible ABI
```

**解决方法**:
- 确保使用 **Qt MinGW 版本**，而不是 MSVC 版本
- 正确路径示例：`C:\Qt\6.5.3\mingw_64`（包含 `mingw`）
- 错误路径示例：`C:\Qt\6.5.3\msvc2019_64`（包含 `msvc`）

---

## 高级用法

### 查看所有可用的 Preset

```bash
# 列出所有配置预设
cmake --list-presets

# 列出所有构建预设
cmake --build --list-presets

# 列出所有打包预设
cpack --list-presets
```

### 自定义配置（CMakeUserPresets.json）

`CMakeUserPresets.json` 用于存储本地机器特定的配置（如 Qt 路径），不会被 Git 跟踪。

**初始化：**
```bash
cp CMakeUserPresets.json.example CMakeUserPresets.json
```

**文件结构：**
| 文件 | 用途 | Git |
|------|------|-----|
| `CMakePresets.json` | 项目通用配置 | ✅ 提交 |
| `CMakeUserPresets.json` | 本地机器配置（Qt 路径等） | ❌ 忽略 |
| `CMakeUserPresets.json.example` | 模板文件 | ✅ 提交 |

**添加自定义 preset：**
```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "macos-release",
      "inherits": "macos-release",
      "cacheVariables": {
        "CMAKE_PREFIX_PATH": "$env{HOME}/Qt/6.8.3/macos"
      }
    },
    {
      "name": "my-dev",
      "inherits": "macos-release",
      "cacheVariables": {
        "CMAKE_PREFIX_PATH": "/custom/path/to/Qt",
        "MY_CUSTOM_OPTION": "ON"
      }
    }
  ]
}
```

### 生成依赖关系图

编译时自动生成 CMake 依赖关系图：

```bash
./build.sh macos-release

# 查看生成的图文件
# build/macos-release/cmake_graph/cmake_graph.dot

# 转换为图片（需要安装 Graphviz）
dot -Tpng build/macos-release/cmake_graph/cmake_graph.dot -o deps.png
```

---

## 贡献指南

欢迎提交 Issue 和 Pull Request！

### 开发流程

1. Fork 本仓库
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -am 'Add some feature'`
4. 推送分支：`git push origin feature/your-feature`
5. 提交 Pull Request

---

## 许可证

查看 `LICENSE` 文件了解详情。

---

## 联系方式

- Email: 47029316@qq.com

---

**祝你编译顺利！** 🎉