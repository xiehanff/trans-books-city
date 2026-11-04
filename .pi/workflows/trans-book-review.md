# trans-book-review

用于审核任意译书 Markdown：内容、排版和原文/OCR 三路只读审核后，由单一修订代理应用有依据的修改，再独立复查并按需同步到维护中心。工作流脚本不包含书籍目录或机器相关的绝对路径；每次运行都通过 `args` 传入路径，因此可用于不同书籍和电脑。

## 运行参数

`target` 和 `sourceRoot` 必填。原版可使用扫描/可选中文本 PDF，也可使用 EPUB；`originalSource` 可选，未提供时原文校对角色使用 `sourceRoot` 中找到的材料。`maintenanceRoot` 可选，不传时跳过同步。模型默认为 GPT-6 Luna，可通过 `model` 覆盖。

```json
{
  "target": "/path/to/translated-book-markdown",
  "sourceRoot": "/path/to/book-source-and-ocr-materials",
  "originalSource": "/path/to/original-book.pdf",
  "maintenanceRoot": "/path/to/book-maintenance-folder",
  "model": "openai-codex/gpt-6-luna"
}
```

EPUB 示例：

```json
{
  "target": "/path/to/translated-book-markdown",
  "sourceRoot": "/path/to/book-source-and-ocr-materials",
  "originalSource": "/path/to/original-book.epub",
  "maintenanceRoot": "/path/to/book-maintenance-folder"
}
```

通过当前 Pi 客户端的 `SubagentWorkflow` 工具运行 `trans-book-review`，并将参数对象传入 `args`。模型参数使用 `provider/modelId` 格式。

## 固定流程

1. **Review**：内容、排版、原文/OCR 三个角色并行只读审阅，避免多个代理同时编辑同一文件。
   - 内容：逻辑、错别字、译文表达、代码逻辑/语法/字符缺失、bug 和代码格式。
   - 排版：Markdown 标题与段落、强调/高亮、代码围栏、公式、时序图、表格、链接等。
   - 原文/OCR：检查漏译、段落和图表/代码/公式遗漏及 OCR 错误，逐项保留来源证据。
2. **Revise**：单一代理合并意见，只应用证据明确的修订；有争议或无法辨认的地方列为待确认。
3. **Verify**：独立检查修订、Markdown 结构及明显回归。
4. **Sync**：仅当指定 `maintenanceRoot` 时，将目标目录根层以数字编号开头的章节 Markdown 同名文件同步到维护中心已有文件；核验 SHA-256。不会删除/新建文件，不覆盖 OCR 原文、图片、README、目录或终审材料。

## PDF 与 EPUB 校对规则

- **PDF**：先检查是否有文本层；扫描页可用 PyMuPDF 等工具渲染页面图像并直接目视核对。OCR 仅用于检索和定位，不能代替原页图像证据。报告 PDF 页码，必要时同时记录印刷页码。
- **EPUB**：只读检查 EPUB 容器、OPF/spine、章节 XHTML 和资源；按原书章节逐一映射译文。特别核对代码块、表格、图片说明、公式及数学标记。报告 EPUB 内文件名和章节/片段位置。
- **两者**：原版和素材只读。对图像分辨率不足或资源本身不可辨认的内容，标为待确认，不猜补；不要将正常翻译差异误判为遗漏。

工作流不会把“无法确认”改成确定结论。页面外观是否符合静态站点的实际渲染，仍需在目标站点预览验证。
