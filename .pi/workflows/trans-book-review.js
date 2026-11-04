export const meta = {
  name: 'trans-book-review',
  description: '对照原版 PDF/EPUB 与素材审核译书，修订、复查后同步章节到维护中心',
  whenToUse: '审核任意译书；通过 args 指定目标目录、原版 PDF/EPUB、素材根目录和维护中心。',
  phases: [
    { title: 'Review', detail: '内容、排版、OCR 三角色独立审阅' },
    { title: 'Revise', detail: '汇总并应用已核实修订' },
    { title: 'Verify', detail: '复查改动并汇报未解决项' },
    { title: 'Sync', detail: '将审核后的章节 Markdown 同步到维护中心' },
  ],
}

const target = args?.target
const sourceRoot = args?.sourceRoot
const model = args?.model ?? 'openai-codex/gpt-6-luna'
const originalSource = args?.originalSource ?? null
const maintenanceRoot = args?.maintenanceRoot ?? null

if (!target) throw new Error('请在 args.target 中指定待审核书籍的 Markdown 目录。')
if (!sourceRoot) throw new Error('请在 args.sourceRoot 中指定原书/OCR/EPUB 素材目录。')

const roles = [
  {
    name: 'content',
    label: '内容审核',
    prompt: `你是译书的内容审核员。通读目标目录中的全部书籍 Markdown，站在读者角度检查确定的问题，但不要编辑文件。报告建议修订：论述逻辑、错别字、翻译表达、代码逻辑/语法/字符缺失、明显 bug、代码格式。不要臆测事实，不要改动无问题内容；代码示例需尽量实际运行或用静态推理核实。保留作者原意及现有文件结构。输出简明列出涉及文件、建议修订及依据和无法确认的问题。`,
  },
  {
    name: 'layout',
    label: '排版审核',
    prompt: `你是 Markdown 排版审核员。通读目标目录全部书籍 Markdown，检查渲染与可读性问题，但不要编辑文件。报告建议修订：标题层级、段落过长或不合理拆分、代码围栏/语言标注、强调或高亮作用域污染、公式、时序图、表格、列表及链接格式。只修 Markdown 结构和明确的排版问题，不改写内容含义；保持仓库现有格式约定。输出涉及文件、建议修订及仍需人工确认的问题。`,
  },
  {
    name: 'ocr',
    label: 'OCR 校对',
    prompt: `你是原书/OCR 校对员。逐章对照目标译文和原版来源：如果 originalSource 是 PDF，区分可提取文本和扫描页面；扫描页按页渲染图像并直接目视检查，OCR 仅作检索辅助。如果是 EPUB，先安全读取容器目录与 spine/章节 XHTML，再按原书章节对照译文，重点检查代码块、表格、图片说明、公式和数学标记是否漏失。也可使用 sourceRoot 中已有的 OCR/提取文本辅助定位，但不要让 OCR 输出代替原书证据。核对是否漏译/漏段，以及图表/代码/公式缺失、字符识别错误。每项问题记录目标文件/行、来源文件及 PDF 页码或 EPUB 章节定位，并给出可核对的原文证据。原版文件和素材只读；对图像分辨率不足或内容本身不可辨认的地方标为待确认，不要猜测；不要把正常翻译差异当作遗漏。`,
  },
]

const reviews = await parallel(roles.map(role => () => agent(
  `${role.prompt}\n\n目标书籍目录：${target}\n原始素材根目录：${sourceRoot}\n原版来源 PDF/EPUB（如提供）：${originalSource ?? '未提供'}\n先确认目录存在并枚举所有相关文件。不要把目标目录之外的文件当作修改对象。`,
  { label: role.label, phase: 'Review', model },
)))

const reviewNotes = reviews.map((result, i) => `## ${roles[i].label}\n${result ?? '审核未返回结果'}`).join('\n\n')

const revisions = await agent(
  `你负责整合三类审核结论并完成最终修订。目标书籍目录：${target}\n原始素材根目录：${sourceRoot}\n\n审核结论：\n${reviewNotes}\n\n只应用有明确依据、且与其他审核结论不冲突的修订；OCR 修订必须能回溯到素材证据。对冲突或证据不足项不要自行猜测，保留原文并列为待确认。所有写入严格限制在目标书籍目录。检查 git diff，保留任务开始前已有的改动，不要覆盖无关内容。完成后汇报实际改动文件、主要修订、未解决问题。`,
  { label: '汇总并修订', phase: 'Revise', model },
)

const verification = await agent(
  `独立复查本次目标目录的 git diff：${target}。验证修订符合审核结论、没有误删或无证据改写；抽查 Markdown 围栏/标题/公式/表格结构及代码片段。不要再做大范围重写；发现明确回归时修复，否则报告问题。报告检查范围、确认的结果及仍需人工核对的 OCR/渲染事项。`,
  { label: '独立复查', phase: 'Verify', model },
)

const sync = maintenanceRoot
  ? await agent(
      `将完成审核的章节 Markdown 同步到维护中心。来源目录：${target}\n目标维护中心：${maintenanceRoot}\n\n仅同步来源目录根层中以数字编号开头的章节 Markdown（文件名匹配正则表达式 ^[0-9]+[-_.].+\\.md$），将已有同名文件复制/覆盖到维护中心根层；不要创建或删除其他文件。若来源章节在维护中心找不到同名文件，跳过并报告。不要同步封面与出版信息目录，也绝不覆盖 OCR原文/、封面与出版信息/、终审材料/、README.md、目录.md 或其他任何素材。复制后对每个同步文件核验 SHA-256 一致，并报告文件清单、跳过项和结果。`,
      { label: '同步维护中心', phase: 'Sync', model },
    )
  : '未配置维护中心路径，已跳过同步；请通过 maintenanceRoot 指定。'

return { target, sourceRoot, originalSource, maintenanceRoot, model, reviews: reviewNotes, revisions, verification, sync }
