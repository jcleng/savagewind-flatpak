# savagewind-flatpak

非官方 **Savage Wind Recomp**（《塞尔达传说：旷野之息》Switch 版静态重编译项目，面向 PC）的 Flatpak 封装。

**构建方式**：直接解包上游官方 AppImage Release 并重新打包为 Flatpak，**不做源码编译**，因此构建快、产物与上游二进制完全一致。

- 上游源码：https://github.com/Revivedx/SavageWindRecomp
- 上游文档（Linux 说明）：https://github.com/Revivedx/SavageWindRecomp/blob/main/docs/linux.md

---

## 下载安装

从本仓库的 [Releases](../../releases) 页面下载 `com.revivedx.SavageWind.flatpak`：

```bash
flatpak install --user com.revivedx.SavageWind.flatpak
flatpak run com.revivedx.SavageWind
```

「Savage Wind Configuration」（设置 / 游戏文件管理界面）在桌面右键菜单的 *Configuration* 动作里，
或直接执行 `flatpak run com.revivedx.SavageWind --config`。

## ⚠️ 重要：需要自备游戏文件

**本包不含任何版权游戏资源、密钥或 DUMP 工具。** 你需要自己合法获得的《旷野之息》**1.9.0** 版：

```
extracted/
  exefs/   main, main.npdm, rtld, sdk, subsdk0
  romfs/   完整 RomFS（本体 + 1.9.0 更新合并）
  aoc1/    可选：第一弹 DLC 的 RomFS
  aoc2/    可选：第二弹 DLC 的 RomFS
```

首次启动时程序会要求选择这个目录，检查版本、复制文件，并**在本机重新编译游戏代码**
（12 线程 CPU 约 6 分钟，另加复制与可选着色器预编译）。上游采取零容忍政策，不提供、不分发任何游戏文件。

运行要求：x86-64 CPU（SSE4.2，有 AVX2 更快）、**Vulkan 1.3** 显卡驱动、16 GB 内存、约 20 GB 空闲磁盘。

## 数据目录

```bash
~/.var/app/com.revivedx.SavageWind/data/savagewind
```

`savagewind.log`、`savagewind.ini`、`save/`（存档）、已编译的游戏库、着色器缓存都在这里。
内层路径与上游 Linux 版 `~/.local/share/savagewind` 完全一致，存档可直接迁移。

## 常见问题

**配置里的 `savagewind.ini` 怎么用？**
手动指定 Vulkan GPU：编辑 `~/.var/app/com.revivedx.SavageWind/data/savagewind/savagewind.ini`，
设 `gpu = 1`（数字看日志里 `[vk] gpu N:` 那几行），重启即可。临时覆盖用环境变量：

```bash
flatpak run --env=SAVAGEWIND_GPU=1 com.revivedx.SavageWind
```

**setup 阶段卡住或报错？**
先看 `~/.var/app/com.revivedx.SavageWind/data/savagewind/savagewind.log`。setup 需要沙箱内可写的临时空间，
包装脚本已把 `TMPDIR` 指向 `${SAVAGEWIND_DATA_DIR}/.tmp`（沙箱内 `/tmp` 是内存 tmpfs，不适合大工程编译）。

---

## 版本对应

| 项目 | 值 |
|---|---|
| 应用 ID | `com.revivedx.SavageWind` |
| 上游版本 | Savage Wind **0.5.0-alpha.5** (Alpha 5) |
| 运行时 | `org.freedesktop.Platform` 25.08 |
| 构建来源 | 上游官方 `Savage_Wind-0.5.0-alpha.5-x86_64.AppImage` |

升级上游版本需要修改两处：
1. `com.revivedx.SavageWind.yaml` 里 `sources[].url` 的版本号和 `sha256`
2. `.github/workflows/build.yml` 里的 `SAVAGEWIND_VERSION`（以及 metainfo 里的 `<release>`）

上游每个 release 的 `digest` 字段就是 sha256，可直接取用。

## 为什么这样做（可行性依据）

封装前对官方 AppImage 0.5.0-alpha.5 做了实际拆包分析：

| 检查项 | 结果 | 影响 |
|---|---|---|
| 二进制 RUNPATH | `$ORIGIN/../lib` | 装到 `/app/bin` + `/app/lib` 即可，无需 patchelf |
| 动态依赖 | 仅 12 个 NEEDED | SDL3 / ffmpeg / shaderc 随包自带；宿主只需 `libvulkan.so.1`、`libz.so.1`、`libstdc++`（runtime 提供） |
| glibc 需求 | 最高 `GLIBC_2.35` | Freedesktop 25.08 满足 |
| `/app` 硬编码路径 | 无 | 不会因只读 `/app` 写入失败 |
| 配置路径 | 遵循 `XDG_DATA_HOME` | Flatpak 自动沙箱化到 `~/.var/app/` |
| 游戏文件是否内含 | 否 | 仅空目录骨架 + 字体 + LUT + 着色器种子列表 |
| 内置工具链 | `usr/bin/zig`（静态）、`usr/bin/swgen` | 必须保留，setup 阶段在本机编译游戏代码，用 $ORIGIN 相对查找 |

AppImage 体积 86 MB，解包后约 320 MB（其中 `zig/` 118 MB，`swgen/` 49 MB，都是 setup 必需，不能裁）。

## 本仓库结构

```
com.revivedx.SavageWind.yaml          # Flatpak 清单（核心）
savagewind-wrapper                    # 启动包装脚本（环境兜底 + 固定工作目录）
com.revivedx.SavageWind.desktop       # 桌面入口（含 Configuration 动作）
com.revivedx.SavageWind.metainfo.xml  # AppStream 元数据
icons/com.revivedx.SavageWind.png     # 图标（1024x1024，取自上游）
.github/workflows/build.yml           # GitHub Actions 构建 + 自动 Release
```

## 本地构建（可选）

本仓库设计为**只在 GitHub Actions 上构建**；若确实需要本地验证，需要 `flatpak-builder` + `org.freedesktop.Sdk//25.08`：

```bash
git clone <this repo>
cd savagewind-flatpak
flatpak-builder --user --install --force-clean build-dir \
  com.revivedx.SavageWind.yaml
flatpak run com.revivedx.SavageWind
```

## 许可

- Flatpak 清单 / 包装脚本：MIT
- Savage Wind Recomp 本体：MIT，见 [上游](https://github.com/Revivedx/SavageWindRecomp)（内含 Zig、swgen 等第三方组件，各自遵循其许可证）
- 本项目与 Nintendo / Revivedx 均无隶属关系
