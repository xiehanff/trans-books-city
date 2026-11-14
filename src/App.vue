<script setup>
import { computed, onMounted, onUpdated, reactive, ref } from 'vue'
import MarkdownIt from 'markdown-it'
import mermaid from 'mermaid'
import markdownItAnchor from 'markdown-it-anchor'
import katex from 'katex'
import 'katex/dist/katex.min.css'
import hljs from 'highlight.js/lib/core'
import bash from 'highlight.js/lib/languages/bash'
import c from 'highlight.js/lib/languages/c'
import cpp from 'highlight.js/lib/languages/cpp'
import csharp from 'highlight.js/lib/languages/csharp'
import css from 'highlight.js/lib/languages/css'
import dockerfile from 'highlight.js/lib/languages/dockerfile'
import fsharp from 'highlight.js/lib/languages/fsharp'
import graphql from 'highlight.js/lib/languages/graphql'
import javascript from 'highlight.js/lib/languages/javascript'
import json from 'highlight.js/lib/languages/json'
import lua from 'highlight.js/lib/languages/lua'
import makefile from 'highlight.js/lib/languages/makefile'
import markdown from 'highlight.js/lib/languages/markdown'
import plaintext from 'highlight.js/lib/languages/plaintext'
import python from 'highlight.js/lib/languages/python'
import sql from 'highlight.js/lib/languages/sql'
import typescript from 'highlight.js/lib/languages/typescript'
import xml from 'highlight.js/lib/languages/xml'
import x86asm from 'highlight.js/lib/languages/x86asm'
import yaml from 'highlight.js/lib/languages/yaml'
import 'highlight.js/styles/github.css'
import coverCsapp from './assets/covers/csapp-third-edition.jpg'
import coverOstep from './assets/covers/ostep.jpg'
import coverSicp from './assets/covers/sicp-2e.jpg'
import coverComputerNetworking from './assets/covers/computer-networking-top-down.jpg'
import coverCraftingInterpreters from './assets/covers/crafting-interpreters.jpg'
import coverHelloAlgo from './assets/covers/hello-algo.jpg'
import coverDiyLlm from './assets/covers/diy-llm.jpg'
import coverStanfordAi from './assets/covers/stanford-ai-course.jpg'
import coverCNotes from './assets/covers/c-notes.jpg'
import coverCppNotes from './assets/covers/cpp-notes.jpg'
import coverPython from './assets/covers/python-tutorial.jpg'
import coverAiMath from './assets/covers/ai-prereq-math.jpg'
import coverAiNn from './assets/covers/ai-prereq-nn.jpg'
import coverAiNanogpt from './assets/covers/ai-prereq-nanogpt.jpg'
import coverAiCuda from './assets/covers/ai-prereq-cuda.jpg'
import coverAiCs336 from './assets/covers/ai-prereq-cs336.jpg'
import coverAlgorithmDesignManual from './assets/covers/algorithm-design-manual.jpg'

;[['asm', x86asm], ['bash', bash], ['c', c], ['cpp', cpp], ['csharp', csharp], ['css', css], ['dockerfile', dockerfile], ['fsharp', fsharp], ['graphql', graphql], ['javascript', javascript], ['json', json], ['jsx', javascript], ['lua', lua], ['makefile', makefile], ['markdown', markdown], ['python', python], ['sql', sql], ['text', plaintext], ['tsx', typescript], ['typescript', typescript], ['yaml', yaml], ['html', xml]].forEach(([name, language]) => hljs.registerLanguage(name, language))

const REPO = 'https://github.com/xiehanff/trans-books-city'
const BOOKS = [
  {
    slug: 'computer-systems-3e',
    title: 'Computer Systems: A Programmer’s Perspective',
    zh: '深入理解计算机系统（原书第 3 版）',
    label: 'COMPUTER SYSTEMS',
    author: 'Randal E. Bryant / David R. O’Hallaron',
    cover: coverCsapp,
    category: 'cs101',
    desc: '从程序表示与执行，到链接、异常控制流、虚拟内存、网络与并发，系统理解计算机系统的软硬件基础。',
  },
  {
    slug: 'operating-systems-three-easy-pieces',
    title: 'Operating Systems: Three Easy Pieces',
    zh: '操作系统导论',
    label: 'OPERATING SYSTEMS',
    author: 'Remzi H. Arpaci-Dusseau / Andrea C. Arpaci-Dusseau',
    cover: coverOstep,
    category: 'cs101',
    desc: '以虚拟化、并发、持久性三大主题贯穿进程、调度、内存、文件系统与分布式，对话式讲解操作系统核心原理。',
  },
  {
    slug: 'structure-and-interpretation-of-computer-programs',
    title: 'Structure and Interpretation of Computer Programs',
    zh: '计算机程序的构造和解释（第 2 版）',
    label: 'SICP',
    author: 'Harold Abelson / Gerald Jay Sussman / Julie Sussman',
    cover: coverSicp,
    category: 'cs101',
    desc: '用 Scheme 讲透过程抽象、数据抽象、模块化与元语言抽象，程序设计的经典之作，裘宗燕译本。',
  },
  {
    slug: 'computer-networking-top-down-6e',
    title: 'Computer Networking: A Top-Down Approach',
    zh: '计算机网络：自顶向下方法（原书第 6 版）',
    label: 'COMPUTER NETWORKING',
    author: 'James F. Kurose / Keith W. Ross',
    cover: coverComputerNetworking,
    category: 'cs101',
    desc: '从应用层一路向下，讲透运输层、网络层与链路层，配 Wireshark 实验与编程作业的自顶向下网络经典教材。',
  },
  {
    slug: 'diy-llm',
    title: 'CS336',
    zh: 'Diy-LLM · 大模型是怎样炼成的',
    kind: '中文教程',
    label: 'LARGE LANGUAGE MODELS',
    author: 'Datawhale 社区',
    cover: coverDiyLlm,
    desc: '从分词器、Transformer、MoE、分布式训练到 Scaling Laws、推理与对齐，按构建 LLM 的工程流程重新编排的 CS336 中文教程。',
    category: 'ai',
  },
  {
    slug: 'stanford-ai-course',
    title: 'CS146S',
    zh: 'The Modern Software Developer',
    kind: 'AI 编程笔记',
    label: 'AI SOFTWARE ENGINEERING',
    author: 'FL-Bin',
    cover: coverStanfordAi,
    desc: '从编程代理的内部原理、上下文工程与 MCP，到代理代码审查与 AI 代码库安全，按 CS146S 课程主线重排的中文笔记。',
    category: 'ai',
  },
  {
    slug: 'c-notes',
    title: 'C',
    zh: 'C 语言笔记',
    kind: '从语法到系统',
    label: 'C PROGRAMMING',
    author: '个人笔记',
    cover: coverCNotes,
    desc: '从程序结构、指针与内存，到链接、汇编、Makefile 与工程实践，按学习顺序整理的中文 C 语言笔记。',
    category: 'programming',
  },
]
const AI_PREREQ_BOOKS = [
  {
    slug: 'mml-book-zh',
    step: '01',
    cover: coverAiMath,
    title: 'Mathematics for Machine Learning',
    zh: '机器学习数学基础 · 中文译本',
    desc: '只读矩阵运算、链式法则与反向传播、高斯与期望方差、梯度下降四块，配套章末题；不必重学整门线代与微积分。',
    meta: 'Datawhale 中文译本 · 官方 PDF 免费 · 约 30 小时',
    url: 'https://github.com/datawhalechina/math-for-ai/',
    category: 'ai-prereq',
  },
  {
    slug: 'nn-zero-to-hero',
    step: '02',
    cover: coverAiNn,
    title: 'Neural Networks: Zero to Hero',
    zh: '从零构建神经网络 · 手写反向传播',
    desc: '用 micrograd 搭出 autograd 引擎，makemore 逐层手动反传，最后手写 GPT；直接对应 CS336 的 tokenization、架构与训练。',
    meta: 'Andrej Karpathy · 8 讲 · 约 45 小时',
    url: 'https://github.com/karpathy/nn-zero-to-hero',
    category: 'ai-prereq',
  },
  {
    slug: 'nanogpt-gpt2',
    step: '03',
    cover: coverAiNanogpt,
    title: 'Build nanoGPT',
    zh: 'Let’s reproduce GPT-2 (124M)',
    desc: '四小时视频逐行写 causal attention、AdamW、warmup + cosine、混合精度与 checkpoint，再改造成 RMSNorm / RoPE / SwiGLU。',
    meta: 'Andrej Karpathy · 视频 + 代码 · 约 55 小时',
    url: 'https://github.com/karpathy/build-nanogpt',
    category: 'ai-prereq',
  },
  {
    slug: 'cuda-mode',
    step: '04',
    cover: coverAiCuda,
    title: 'CUDA MODE',
    zh: 'GPU 性能与 Triton 实战',
    desc: '从 profiling、内存层级到 kernel 优化与 FlashAttention / Triton，边看边练，衔接 CS336 作业 2 的 kernel 与并行训练。',
    meta: 'GPU MODE 社区 · 讲义与 Colab · 60–75 小时',
    url: 'https://github.com/gpu-mode/lectures',
    category: 'ai-prereq',
  },
  {
    slug: 'cs336-assignments',
    step: '05',
    cover: coverAiCs336,
    title: 'CS336 Assignments',
    zh: '语言模型从零实现 · 官方作业',
    desc: '作业 4 的 Common Crawl 清洗与 MinHash 去重、作业 5 的 SFT 与 GRPO / DPO 对齐，作业说明本身即教程，几乎不给脚手架。',
    meta: 'Stanford · GitHub · 作业 1–5 · 需 GPU 与数据磁盘',
    url: 'https://github.com/stanford-cs336',
    category: 'ai-prereq',
  },
]
const EXTERNAL_BOOKS = [
  {
    slug: 'algorithm-design-manual-pdf',
    title: 'The Algorithm Design Manual',
    zh: '算法设计指南（第 2 版）· 原书扫描版',
    cover: coverAlgorithmDesignManual,
    desc: '中文译本原书扫描 PDF，377 页，含全部插图、习题与参考文献，适合完整阅读与对照。',
    meta: '清华大学出版社 · 377 页 · 站内 PDF',
    url: 'books/the-algorithm-design-manual/algorithm-design-manual-2e-zh.pdf',
    category: 'cs101',
  },
  {
    slug: 'craftinginterpreters-zh',
    title: 'Crafting Interpreters',
    zh: '手撸解释器教程 · 中文译本',
    cover: coverCraftingInterpreters,
    desc: '用 Java 和 C 从零实现 Lox 语言的两种解释器：树遍历的 jlox 与字节码虚拟机 clox，一路做到垃圾回收与优化。',
    meta: 'Robert Nystrom · 30 章 · 独立站点',
    url: 'https://xiehanff.github.io/craftinginterpreters_zh/',
    category: 'cs101',
  },
  {
    slug: 'hello-algo',
    title: 'Hello 算法',
    zh: '动画图解数据结构与算法教程',
    cover: coverHelloAlgo,
    desc: '动画图解、一键运行的数据结构与算法教程，从复杂度分析、数据结构到树、图与动态规划，附十余种语言的代码实现。',
    meta: '靳宇栋 · 18 章 · 独立站点',
    url: 'https://www.hello-algo.com/',
    category: 'cs101',
  },
  {
    slug: 'cpp-notes',
    title: 'C++',
    zh: 'Learn C++ · 从入门到现代 C++',
    cover: coverCppNotes,
    desc: '系统讲解 C++ 语法、类与对象、继承、多态、模板与 STL 的免费教程，适合从 C 语言过渡到现代 C++。',
    meta: 'learncpp.com · 站外教程',
    url: 'https://www.learncpp.com/',
    category: 'programming',
  },
  {
    slug: 'python-tutorial',
    title: 'Python',
    zh: '官方教程 · 从语法到标准库',
    cover: coverPython,
    desc: 'Python 官方教程：解释器、数据类型、控制流、函数、模块、异常、类与标准库；适合有其他语言基础的学习者。',
    meta: 'docs.python.org · 中文官方教程',
    url: 'https://docs.python.org/zh-cn/3/tutorial/index.html',
    category: 'programming',
  },
]
const totalBooks = BOOKS.length + EXTERNAL_BOOKS.length + AI_PREREQ_BOOKS.length
const CATEGORIES = [
  { key: 'cs101', title: 'CS101', desc: '计算机基础' },
  { key: 'programming', title: '编程语言', desc: 'C / C++ 与系统底层' },
  { key: 'ai-prereq', title: 'AI 前置', desc: 'CS336 前置：按 01–05 顺序完成', note: '完成 01–05 后进入下方 CS336；05 是桥接作业，可与 CS336 正文交替进行。' },
  { key: 'ai', title: 'AI', desc: '人工智能与大模型', note: 'CS336 先完成上方 AI 前置 01–05；CS146S 从 AI 编程与代理工程切入，前置是 Python 与软件工程基础，不要求 01–05。' },
]
const booksIn = (category) => BOOKS.filter((book) => book.category === category)
const externalBooksIn = (category) => category === 'ai-prereq'
  ? AI_PREREQ_BOOKS
  : EXTERNAL_BOOKS.filter((book) => book.category === category)

const files = import.meta.glob('../books/*/*.md', { query: '?raw', import: 'default' })
const docs = reactive(Object.entries(files)
  .map(([path, load]) => {
    const [, , slug, filename] = path.split('/')
    const file = decodeURIComponent(filename)
    const number = file.match(/^(\d+)/)
    const title = file.replace(/^\d+-/, '').replace(/\.md$/, '').replace(/-/g, ' ')
    return { book: slug, id: file, title, load, number: number ? Number(number[1]) : 999, file }
  })
  .sort((a, b) => a.number - b.number || a.file.localeCompare(b.file, 'zh-CN')))
const bookDocs = (slug) => docs.filter((doc) => doc.book === slug)
BOOKS.forEach((book) => { book.chapters = bookDocs(book.slug) })

const contentById = reactive({})
const loadingIds = reactive(new Set())
const loadErrors = reactive({})
const pendingLoads = new Map()
const docKey = (doc) => `${doc.book}/${doc.file}`
const navTitle = (doc) => {
  if (doc.book !== 'the-algorithm-design-manual' || doc.number < 1 || doc.number > 10) return doc.title
  const chapterNumber = Number(doc.file.match(/^(\d+)/)?.[1])
  return chapterNumber && !new RegExp(`第\\s*${chapterNumber}\\s*章`).test(doc.title)
    ? `第${chapterNumber}章 ${doc.title}`
    : doc.title
}
async function loadDoc(doc) {
  const key = docKey(doc)
  if (contentById[key] !== undefined) return contentById[key]
  if (!pendingLoads.has(key)) {
    delete loadErrors[key]
    loadingIds.add(key)
    const pending = doc.load()
      .then((content) => {
        contentById[key] = content
        return content
      })
      .catch((error) => {
        loadErrors[key] = '章节加载失败，请重试。'
        throw error
      })
      .finally(() => {
        loadingIds.delete(key)
        pendingLoads.delete(key)
      })
    pendingLoads.set(key, pending)
  }
  return pendingLoads.get(key)
}

const slugify = (value) => value.trim().toLowerCase().replace(/[^\p{L}\p{N}\s-]/gu, '').replace(/\s+/g, '-')
const INLINE_MATH_BOOKS = new Set(['diy-llm', 'the-algorithm-design-manual'])
const md = new MarkdownIt({
  html: true,
  linkify: true,
  typographer: true,
  highlight(code, language) {
    if (language && hljs.getLanguage(language)) return `<pre class="hljs"><code>${hljs.highlight(code, { language }).value}</code></pre>`
    return `<pre class="hljs"><code>${md.utils.escapeHtml(code)}</code></pre>`
  },
}).use(markdownItAnchor, { permalink: false, slugify })
function mathInline(state, silent) {
  if (!INLINE_MATH_BOOKS.has(state.env.doc?.book)) return false
  const start = state.pos
  if (state.src.charCodeAt(start) !== 0x24 || state.src.charCodeAt(start + 1) === 0x24) return false
  let end = start + 1
  while ((end = state.src.indexOf('$', end)) !== -1) {
    if (state.src.charCodeAt(end - 1) !== 0x5c) break
    end += 1
  }
  if (end === -1) return false
  const raw = state.src.slice(start + 1, end)
  const content = raw.trim()
  if (!content || content.includes('\n')) return false
  if (/^\s|\s$/.test(raw) && !/[\\^_={}]/.test(content)) return false
  if (!silent) {
    const token = state.push('math_inline', 'math', 0)
    token.content = content
    token.markup = '$'
  }
  state.pos = end + 1
  return true
}
md.inline.ruler.after('backticks', 'math_inline', mathInline)
md.renderer.rules.math_inline = (tokens, idx) => katex.renderToString(tokens[idx].content, { displayMode: false, throwOnError: false })
function renderMarkdown(content, doc) {
  const math = []
  const linkSafeContent = content.replace(
    /(https?:\/\/[A-Za-z0-9\-._~:/?#\[\]@!$&'()*+,;=%]+)(?=[\u3400-\u9fff，。；：！？、）》】」』])/g,
    '$1 ',
  )
  const source = linkSafeContent.replace(/\$\$([\s\S]+?)\$\$/g, (_, expression) => {
    math.push(katex.renderToString(expression.trim(), { displayMode: true, throwOnError: false }))
    return `\n\n@@MATH${math.length - 1}@@\n\n`
  })
  return md.render(source, { doc })
    .replace(/<p>@@MATH(\d+)@@<\/p>/g, (_, index) => `<div class="math-display">${math[Number(index)]}</div>`)
    .replace(/<img\b(?![^>]*\bloading=)/gi, '<img loading="lazy" decoding="async"')
}
mermaid.initialize({ startOnLoad: false, theme: 'neutral', fontFamily: 'inherit', flowchart: { curve: 'basis' } })
const defaultFence = md.renderer.rules.fence.bind(md)
md.renderer.rules.fence = (tokens, idx, options, env, self) => {
  const token = tokens[idx]
  if (token.info.trim().split(/\s+/)[0] === 'mermaid') {
    return `<div class="mermaid">${md.utils.escapeHtml(token.content)}</div>`
  }
  return defaultFence(tokens, idx, options, env, self)
}
md.core.ruler.push('book_callouts', (state) => {
  const labels = { NOTE: '提示', TIP: '建议', IMPORTANT: '重点', WARNING: '注意', CAUTION: '警告' }
  for (let i = 0; i < state.tokens.length; i++) {
    const token = state.tokens[i]
    if (token.type !== 'inline' || !token.content.startsWith('[!')) continue
    const match = token.content.match(/^\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\]\s*/)
    if (!match) continue
    token.content = token.content.slice(match[0].length)
    if (token.children?.[0]?.type === 'text') token.children[0].content = token.children[0].content.replace(match[0], '')
    const open = state.tokens.slice(0, i).reverse().find((item) => item.type === 'blockquote_open')
    if (open) {
      open.attrSet('class', `callout callout-${match[1].toLowerCase()}`)
      open.attrSet('data-label', labels[match[1]])
    }
  }
})
const defaultLinkOpen = md.renderer.rules.link_open || ((tokens, idx, options, env, self) => self.renderToken(tokens, idx, options))
md.renderer.rules.link_open = (tokens, idx, options, env, self) => {
  const token = tokens[idx]
  const hrefIndex = token.attrIndex('href')
  if (hrefIndex >= 0 && token.attrs[hrefIndex][1].endsWith('.md')) {
    const filename = decodeURIComponent(token.attrs[hrefIndex][1].split('/').pop())
    if (docs.some((doc) => doc.id === filename)) token.attrs[hrefIndex][1] = `#${encodeURIComponent(`${env.doc.book}/${filename}`)}`
  }
  return defaultLinkOpen(tokens, idx, options, env, self)
}

const query = ref('')
const activeBookSlug = ref('')
const activeId = ref('')
const menuOpen = ref(false)
const codePath = ref('')
const codeContent = ref('')
const codeError = ref('')
const CODE_PAGES = {
  'assignment1-basics': '17-作业1-基础实现.md',
  'assignment2-systems': '18-作业2-系统优化.md',
  'assignment3-scaling': '19-作业3-缩放定律.md',
  'assignment4-data': '20-作业4-数据处理.md',
  'assignment5-alignment': '21-作业5-对齐.md',
  'assignment6-evaluation': '23-作业6-模型评估.md',
}
const codeRawHref = computed(() => `${import.meta.env.BASE_URL}${codePath.value}`)
const codeBackLabel = computed(() => codePath.value.split('/')[1] === 'c-notes' ? '← 返回 C 笔记' : '← 返回作业')
const codeBackHref = computed(() => {
  const parts = codePath.value.split('/')
  if (parts[1] === 'c-notes') return `#${encodeURIComponent('c-notes/00-大纲与目录.md')}`
  return `#${encodeURIComponent(`diy-llm/${CODE_PAGES[parts[3]] || '00-前言.md'}`)}`
})
const codeHtml = computed(() => {
  if (!codeContent.value) return ''
  const ext = codePath.value.split('.').pop().toLowerCase()
  const lang = { py: 'python', ipynb: 'json', sh: 'bash', toml: 'ini', yaml: 'yaml', yml: 'yaml', json: 'json', md: 'markdown' }[ext]
  return lang && hljs.getLanguage(lang)
    ? hljs.highlight(codeContent.value, { language: lang }).value
    : md.utils.escapeHtml(codeContent.value)
})
async function showCode(path) {
  codePath.value = path
  codeContent.value = ''
  codeError.value = ''
  activeBookSlug.value = ''
  activeId.value = ''
  query.value = ''
  menuOpen.value = false
  window.scrollTo(0, 0)
  try {
    const response = await fetch(`${import.meta.env.BASE_URL}${path}`)
    if (!response.ok) throw new Error(String(response.status))
    codeContent.value = await response.text()
  } catch {
    codeError.value = '代码文件加载失败，请打开原始文件查看。'
  }
}
const activeBook = computed(() => BOOKS.find((book) => book.slug === activeBookSlug.value) || null)
const currentDocs = computed(() => (activeBook.value ? activeBook.value.chapters : []))
const active = computed(() => currentDocs.value.find((doc) => doc.id === activeId.value) || currentDocs.value[0])
const currentIndex = computed(() => currentDocs.value.findIndex((doc) => doc.id === active.value?.id))
const previousDoc = computed(() => currentIndex.value > 0 ? currentDocs.value[currentIndex.value - 1] : null)
const nextDoc = computed(() => currentIndex.value >= 0 && currentIndex.value < currentDocs.value.length - 1 ? currentDocs.value[currentIndex.value + 1] : null)
const filteredDocs = computed(() => currentDocs.value.filter((doc) => !query.value || `${doc.title} ${doc.file}`.toLowerCase().includes(query.value.toLowerCase())))
const activeDocKey = computed(() => active.value ? docKey(active.value) : '')
const groups = computed(() => {
  const sections = [
    { title: '开始阅读', test: (doc) => doc.number === 0 },
    { title: '正文', test: (doc) => doc.number > 0 && !/附录|勘误|API时效|术语表|更多资源|参考文献|作业/.test(doc.file) },
    { title: '附录', test: (doc) => doc.number > 0 && /附录|勘误|API时效|术语表|更多资源|参考文献|作业/.test(doc.file) },
  ]
  return sections.map((section) => ({ ...section, docs: filteredDocs.value.filter(section.test) })).filter((section) => section.docs.length)
})
const rendered = computed(() => {
  const content = activeDocKey.value ? contentById[activeDocKey.value] : ''
  return content && active.value ? renderMarkdown(content, active.value) : ''
})
async function renderMermaidBlocks() {
  const nodes = document.querySelectorAll('.mermaid:not([data-processed])')
  if (nodes.length) mermaid.run({ nodes, suppressErrors: true }).catch((e) => console.error('mermaid run failed:', e))
}
onMounted(renderMermaidBlocks)
onUpdated(renderMermaidBlocks)

function activateDoc(doc, scrollToTop = true) {
  codePath.value = ''
  if (doc.book !== activeBookSlug.value) query.value = ''
  activeBookSlug.value = doc.book
  activeId.value = doc.id
  menuOpen.value = false
  if (scrollToTop) window.scrollTo(0, 0)
  loadDoc(doc).catch(() => {})
}
function select(doc) {
  location.hash = encodeURIComponent(`${doc.book}/${doc.file}`)
  activateDoc(doc)
}
function retryLoad() {
  if (active.value) loadDoc(active.value).catch(() => {})
}
function openBook(book) {
  if (book.chapters[0]) select(book.chapters[0])
}
function goHome() {
  location.hash = ''
  codePath.value = ''
  activeBookSlug.value = ''
  activeId.value = ''
  query.value = ''
  menuOpen.value = false
  window.scrollTo(0, 0)
}
// hash 不是响应式的，必须监听 hashchange：正文里的章节链接、浏览器前进后退都走这里
function syncFromHash(scrollToTop = true) {
  const id = decodeURIComponent(location.hash.slice(1)).replace(/^\//, '')
  if (!id) {
    if (activeBookSlug.value || codePath.value) goHome()
    return
  }
  if (id.startsWith('code/')) {
    showCode(id.slice(5))
    return
  }
  codePath.value = ''
  const [slug, file] = id.split('/')
  const book = BOOKS.find((item) => item.slug === slug)
  const doc = book ? book.chapters.find((item) => item.file === file) : null
  if (!doc) return
  activateDoc(doc, scrollToTop)
}
syncFromHash(false)
window.addEventListener('hashchange', () => syncFromHash())
window.addEventListener('keydown', (event) => {
  if ((event.metaKey || event.ctrlKey) && event.key.toLowerCase() === 'k') {
    event.preventDefault()
    document.querySelector('.search input')?.focus()
  }
})
</script>

<template>
  <header class="topbar">
    <button class="menu-toggle" aria-label="切换目录" @click="menuOpen = !menuOpen">☰</button>
    <a class="brand" href="#" @click.prevent="goHome"><span class="brand-mark">T</span><span>Trans Books City</span></a>
    <div class="top-actions"><span class="edition">中文译本与教程 · 精选技术书籍</span><a :href="REPO" target="_blank" rel="noreferrer" aria-label="GitHub">GitHub ↗</a></div>
  </header>
  <main v-if="codePath" class="code-page">
    <div class="code-wrap">
      <div class="code-head">
        <a :href="codeBackHref">{{ codeBackLabel }}</a>
        <span class="code-path">{{ codePath }}</span>
        <a :href="codeRawHref" target="_blank" rel="noreferrer">原始文件 ↗</a>
      </div>
      <p v-if="codeError" class="article-status article-error" role="alert">{{ codeError }}</p>
      <p v-else-if="!codeContent" class="article-status" role="status">正在加载代码…</p>
      <pre v-else class="code-view"><code class="hljs" v-html="codeHtml"></code></pre>
    </div>
  </main>
  <main v-else-if="!activeBook" class="home">
    <div class="home-wrap">
      <section class="home-hero">
        <h1>Trans Books City</h1>
        <p>{{ totalBooks }} 个精选技术书籍与开源课程。本站内书籍逐章整理评审，站外译本与开源教程直达独立站点。</p>
      </section>
      <section v-for="category in CATEGORIES" :key="category.key" class="book-section">
        <div class="section-head">
          <h2>{{ category.title }}</h2>
          <span>{{ category.desc }}</span>
        </div>
        <p v-if="category.note" class="section-note">{{ category.note }}</p>
        <div class="book-grid">
          <button v-for="book in booksIn(category.key)" :key="book.slug" class="book-card" @click="openBook(book)">
            <img class="book-cover" :src="book.cover" :alt="`《${book.title}》封面`" loading="lazy" />
            <div class="book-info">
              <h2 class="book-title">{{ book.title }}</h2>
              <p class="book-zh">{{ book.zh }}{{ book.kind ? ` · ${book.kind}` : ' · 中文版' }}</p>
              <p class="book-desc">{{ book.desc }}</p>
              <p class="book-meta">{{ book.author }} · {{ book.chapters.length }} 章</p>
            </div>
          </button>
          <a
            v-for="book in externalBooksIn(category.key)"
            :key="book.slug"
            class="book-card book-card-external"
            :href="book.url"
            target="_blank"
            rel="noreferrer"
            :aria-label="`${book.zh}，在新窗口打开${book.url.startsWith('http') ? '独立站点' : '站内 PDF'}`"
          >
            <img class="book-cover" :src="book.cover" :alt="`《${book.title}》封面`" loading="lazy" />
            <div class="book-info">
              <h2 class="book-title">{{ book.title }}</h2>
              <p class="book-zh">{{ book.zh }}</p>
              <p class="book-desc">{{ book.desc }}</p>
              <p class="book-meta"><span v-if="book.step" class="book-step">{{ book.step }} ·</span>{{ book.meta }} <span class="book-external">↗</span></p>
            </div>
          </a>
        </div>
      </section>
      <footer class="home-foot">内容仅供学习交流，版权归原作者所有 · <a :href="REPO">在 GitHub 上查看 ↗</a></footer>
    </div>
  </main>
  <div v-else class="shell">
    <aside class="sidebar" :class="{ 'sidebar-open': menuOpen }">
      <a class="back-link" href="#" @click.prevent="goHome">← 返回书架</a>
      <div class="search"><span>⌕</span><input v-model="query" placeholder="搜索章节…" aria-label="搜索章节" /><kbd>⌘ K</kbd></div>
      <nav aria-label="章节目录">
        <section v-for="group in groups" :key="group.title" class="nav-group">
          <h2>{{ group.title }}</h2>
          <button v-for="doc in group.docs" :key="doc.id" class="nav-item" :class="{ active: active.id === doc.id }" @click="select(doc)">{{ navTitle(doc) }}</button>
        </section>
      </nav>
      <div class="sidebar-foot">{{ activeBook.title }} · {{ activeBook.kind || '中文译本' }}<br /><a :href="REPO">在 GitHub 上查看 ↗</a></div>
    </aside>
    <main class="main">
      <div class="article-wrap">
        <div class="article-meta"><span>{{ activeBook.label }}</span><a :href="`${REPO}/blob/main/books/${activeBook.slug}/${encodeURIComponent(active.file)}`" target="_blank" rel="noreferrer">编辑此页 ↗</a></div>
        <p v-if="loadingIds.has(activeDocKey)" class="article-status" role="status">正在加载章节…</p>
        <div v-else-if="loadErrors[activeDocKey]" class="article-status article-error" role="alert">
          <span>{{ loadErrors[activeDocKey] }}</span>
          <button type="button" @click="retryLoad">重试</button>
        </div>
        <article v-else class="markdown-body" v-html="rendered" />
        <nav v-if="previousDoc || nextDoc" class="article-pagination" aria-label="章节导航">
          <button v-if="previousDoc" class="pagination-card pagination-previous" @click="select(previousDoc)">
            <span class="pagination-label">← 上一章节</span>
            <span class="pagination-title">{{ previousDoc.title }}</span>
          </button>
          <span v-else />
          <button v-if="nextDoc" class="pagination-card pagination-next" @click="select(nextDoc)">
            <span class="pagination-label">下一章节 →</span>
            <span class="pagination-title">{{ nextDoc.title }}</span>
          </button>
        </nav>
        <footer class="article-footer"><span>内容仅供学习交流，版权归原作者所有</span><a :href="REPO">GitHub ↗</a></footer>
      </div>
    </main>
  </div>
</template>
