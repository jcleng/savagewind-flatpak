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

**界面中文显示成问号/方块，或游戏内中文乱码/显示成日文？**

中文出现在**三个不同的界面**上，由**三种不同的字体入口**控制，所以本包内置**两种** CJK 字体：

| 界面 | 环境变量 | 字体 | 轮廓 |
|---|---|---|---|
| 启动器 / Configuration 窗口 | `SAVAGEWIND_LAUNCHER_FONT` | Noto Sans SC | CFF |
| 游戏内叠加菜单（按 Tab 弹出的设置窗） | `SAVAGEWIND_MENU_FONT` | Noto Sans SC | CFF |
| **游戏内文本**（HUD / 对话 / 游戏菜单） | `SAVAGEWIND_FONT` | **文泉驿微米黑** | **TrueType** |

> 为什么游戏内文本必须用另一种字体：游戏内文本走 Switch 的 `nn::pl` 系统共享字体，
> **游戏自己的字体引擎只能读 TrueType(glyf) 轮廓，遇到 CFF(.otf) 会崩溃**
> （上游 `runtime/hle/pl.c` 注释与 `docs/linux.md` 都写明）。所以这一路用文泉驿微米黑
> （TrueType，含简繁日韩）。这也解释了「设成中文后下次打不开」：如果拿 `.otf` 当
> `SAVAGEWIND_FONT`，重启时会在加载字体那一步崩溃。

自查：

```bash
flatpak run --command=sh com.revivedx.SavageWind -c '
  ls -l /app/share/fonts/noto-cjk/ /app/share/fonts/wqy/ &&
  env | grep -E "SAVAGEWIND_(LAUNCHER_|MENU_)?FONT" &&
  grep -nE "SAVAGEWIND_(LAUNCHER_|MENU_)?FONT" /app/bin/savagewind-wrapper'
```

- 文件缺失 → 装的是旧版包体，升级到最新 Release。
- 文件存在但仍乱码 → 把 `~/.var/app/com.revivedx.SavageWind/data/savagewind/savagewind.log`
  （和上一次运行的 `savagewind.prev.log`）附到 issue；日志里 `[pl] system font ...` 那行会说明游戏实际加载的是哪个字体。

已知限制：游戏内文本用文泉驿微米黑，收录 GBK 常用汉字与假名、谚文，**生僻字可能缺**；
ImGui 界面用 Noto Sans SC（简体子集，不含韩文 Hangul，但含简繁汉字与假名）。
要换字体时记住上面的硬约束：**给 `SAVAGEWIND_FONT` 的必须是 TrueType(glyf) 字体**。

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

## 构建流程说明

全部构建都在 GitHub Actions（`.github/workflows/build.yml`）完成，本机不需要装 flatpak：

1. **Prefetch upstream AppImage** —— 先用带重试的 `curl` 把上游 AppImage 下载到 workspace 并校验 sha256。
   上游 release 资产所在的 GitHub objects CDN 会偶发返回 `504`，而 flatpak-builder 的内置下载器遇到这种抖动
   会直接让整个构建失败，所以这一步单独做，最多重试 10 轮。
2. **Patch manifest to local source** —— 把清单里的 remote `url`/`sha256` 就地换成 `path: upstream.AppImage`，
   让 flatpak-builder 只吃本地文件。（仓库里的清单始终保持带 url + sha256 的“可复现”形态。）
3. **Build Flatpak** —— `flatpak-builder` 在 `ghcr.io/flathub-infra/flatpak-github-actions:freedesktop-25.08`
   容器里构建并 `build-bundle` 产出 `.flatpak`。
4. **Verify bundle** —— 实际 `flatpak install --bundle` 一遍，确认 bundle 能被识别，再卸载。
5. **Compute checksum / Generate release notes** → **Upload artifact**（保留 14 天）→ **Release**（push 即自动建 Release，
   产物附 SHA256）。

## 中文显示：三块界面、两个字体入口

先说结论：**中文出现在三块界面上，其中“游戏内文本”和 ImGui 界面用的是完全不同的字体管线**，
所以要两个字体。早期版本只修了 ImGui 那一半（`SAVAGEWIND_MENU_FONT`），所以“游戏界面中文乱码”
一直还在。

| 界面 | 打开方式 | 技术栈 | 环境变量 | 字体 |
|---|---|---|---|---|
| 启动器 / Configuration | 启动时 / `--config` | Dear ImGui | `SAVAGEWIND_LAUNCHER_FONT` | Noto Sans SC (.otf) |
| 游戏内叠加菜单 | 游戏里按 Tab | Dear ImGui | `SAVAGEWIND_MENU_FONT` | Noto Sans SC (.otf) |
| **游戏内文本** | HUD / 对话 / 游戏菜单 | **`nn::pl` 系统共享字体** | **`SAVAGEWIND_FONT`** | **文泉驿微米黑 (.ttc)** |

### 为什么不能依赖硬编码路径

程序自带的 CJK 回退字体在 `ui_merge_fallback_font` 里按固定路径逐个 `fopen`
（简体为 `zh_hans_fonts`）：`C:/Windows/Fonts/msyh.ttc` → `simsun.ttc` →
`/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc` →
`/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc` → `PingFang.ttc`。

在 Flatpak 沙箱里实测确认这几条 Linux 路径**永远不可用**：

| 检查 | 实测结果 |
|---|---|
| 写 `/usr/share/fonts` | `Read-only file system` |
| `/app/share/fonts` 是否出现在 `/usr/share/fonts` | **不会**，只被 fontconfig 收录（`/etc/fonts/conf.d/50-flatpak.conf`） |
| `--filesystem=` 能否映射到 `/usr/share/fonts` | 不能，只能落到 `/run/host/...` |
| Flathub 是否有字体 runtime 扩展 | 没有，仓库索引里无 `org.freedesktop.Platform.fonts` |

所以本包不碰那些硬编码路径，而是用上游自带的**三个环境变量**直接把真实路径给程序（从二进制里
能找到这三个名字）：包装脚本把它们指向 `/app/share/fonts/` 下的字体。

### 为什么游戏内文本必须单独用 TrueType 字体

这是最容易被忽略、也是“设了中文后下次打不开”的根源：

- 游戏内文本走 Switch 的系统共享字体（`nn::pl`），上游在 `runtime/hle/pl.c` 里用
  `SAVAGEWIND_FONT` 指定字体文件；
- **游戏自己的字体引擎只能读 TrueType(glyf) 轮廓，遇到 CFF(.otf) 会崩溃**
  （`runtime/hle/pl.c` 注释与 `docs/linux.md` 都写明）；
- Noto Sans CJK（包括我们给 ImGui 用的 `NotoSansSC-Regular.otf`）是 **CFF/`OTTO`** 字体，
  拿去给 `SAVAGEWIND_FONT` 就是崩溃；
- 文泉驿微米黑是 **TrueType/`glyf`**，face 0 收录 GBK 汉字 + 假名 + 谚文，约 5 MB，
  Apache-2.0（可再分发），是游戏字体引擎能安全加载的最小可用中文字体。

`pl.c` 的 `ttc_face0()` 会把 `.ttc` 的 **face 0** 抽成独立 sfnt，所以本包用的是 wenquanyi
的 「WenQuanYi Micro Hei」（非 Mono）那个 face——已验证是 glyf 且含中日韩字形。

### CI 怎么防止回退

`Verify CJK fonts inside the sandbox` 这一段在装好的沙箱里：

1. 确认 `/app/share/fonts/noto-cjk/` 与 `/app/share/fonts/wqy/` 两个字体都在，wrapper 三个变量都设了；
2. 对 `SAVAGEWIND_FONT` 指向的 `.ttc` 做**结构断言**：face 0 必须有 `glyf` 表、**不能有 `CFF ` 表**、
   且 `cmap` 含 `中设置日あ가`——任一条失败就中断构建（防止以后又把 `.otf` 塞给游戏内文本而崩溃）；
3. 用 stb_truetype（ImGui 同款解码器）把两个字体都真加载一遍并查字形。

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
icons/com.revivedx.SavageWind.png     # 图标（512x512，取自上游并缩放）
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
