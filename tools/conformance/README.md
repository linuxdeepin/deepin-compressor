# 归档后端一致性观测器（conformance）

用于 `3rdparty/` 后端自研替换的**行为契约测试**：把任意后端在任意样本上的行为录成可版本化的 JSON（golden），替换前后跑同一份观测，逐字段比对。

配套方案文档：`docs/测试优先的替换验证方案.md`

## 为什么不用现有 UT

现有 `tests/UnitTest` 是白盒单函数测试，换实现后要跟着重写，**没有守门作用**。
这里测的是"输入压缩包 → 操作 → 可观测结果"，与实现无关，旧实现和新实现跑同一套用例。

## 快速开始

```bash
# 1. 构建（不进入默认构建）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CONFORMANCE=ON
make -C build -j conformance
make -C build -j libzipplugin readwritelibarchiveplugin readonlylibarchiveplugin \
                  gzplugin bz2plugin xzplugin minizipplugin cli7zplugin clirarplugin libpigzplugin

# 2. 生成语料（需要 zip / tar / 7z / bsdtar）
tools/conformance/make_corpus.sh

# 3. 用【当前实现】录基线（必须在动代码之前做）
tools/conformance/run_matrix.py record --build build

# 4. CI 闸门
tools/conformance/run_matrix.py verify --build build

# 5. 任意两份观测的差异报告
tools/conformance/run_matrix.py diff a.json b.json
```

环境依赖：`LD_LIBRARY_PATH` 会自动指向 `<build>/3rdparty/interface`；`QT_QPA_PLATFORM=offscreen` 自动设置。

## 观测面

一次观测输出一份 JSON，包含：

| 字段 | 内容 |
|---|---|
| `result` | `finish`（PluginFinishType）、`errorType`、`async`（`waitForFinished()` 的真实语义）、`waitTimedOut` |
| `archiveData` | `qSize` `compressedSize` `comment` `isListEncrypted` + 全量条目（name/fullPath/isDir/size/mtime/alias/index）+ 首层条目 |
| `signals` | 进度序列（含单调性）、当前文件名、完成/取消计数、**Query 序列**（类型 + 应答） |
| `fs` | 解压落盘快照：类型、sha256、权限、mtime、软链相对目标 |
| `produced` | 压缩产物条目（由 **bsdtar** 独立列出，避免用被测后端验证自己） |

## 比对规则

- **FAIL（语义）**：返回值、错误码、条目集合、大小、注释、加密标记、落盘内容/权限/软链、产物条目
- **WARN（容忍）**：进度点数与抖动、当前文件名采样
- **用例级放宽**：如 `gz.extract` 的 `fs.mtime`（单文件插件不恢复 mtime，实测），必须逐用例声明，不许全局放过

## 增加用例

编辑 `run_matrix.py` 的 `CASES`：

```python
dict(
    name="libzip.add.zip",           # golden 文件名
    backend="libzip",                # 对应 SO 表里的键
    op="add",                        # list|test|extract|add
    sample=f"{CORPUS}/sample.zip",
    respond="overwrite",             # overwrite|skip|cancel
    extra=["--input", "/path/f"],    # 透传给观测器的额外参数
    note="守什么语义",
    unstable=["fs.mtime"],           # 该用例内声明不稳定字段
)
```

## CI 接入

```yaml
- name: archive backend conformance
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CONFORMANCE=ON
    make -C build -j$(nproc) conformance
    make -C build -j$(nproc) libzipplugin readwritelibarchiveplugin readonlylibarchiveplugin \
                          gzplugin bz2plugin xzplugin minizipplugin cli7zplugin clirarplugin libpigzplugin
    tools/conformance/make_corpus.sh
    tools/conformance/run_matrix.py verify --build build   # 非 0 即阻断
```

## 已知缺口

| 缺口 | 影响 | 需要什么 |
|---|---|---|
| `Query` 密码无法注入（`m_data["password"]` 只能由对话框写入） | 加密包**成功解压**路径无法自动化，目前只覆盖"取消"分支 | 在 `queries.h` 增加 `setQueryData(key, value)` 之类的测试注入点 |
| 语料缺 rar / 分卷 / zip64 / GBK 名 / 超长名 / 恶意包 | 长尾行为未被守住 | 按方案文档 §7.2 扩充 `make_corpus.sh` |
| `clipzip`（pzip）未覆盖 | 该后端依赖 `/usr/lib/deepin-compressor/pzip` 外部可执行 | 安装 pzip 或让它支持库内路径 |
| 部分解压（`extractFiles(files)` 非空）未覆盖 | 提取选中条目的路径未守住 | 观测器增加 `--entry` 选择能力 |
