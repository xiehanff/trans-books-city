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
import csharp from 'highlight.js/lib/languages/csharp'
import css from 'highlight.js/lib/languages/css'
import dockerfile from 'highlight.js/lib/languages/dockerfile'
import fsharp from 'highlight.js/lib/languages/fsharp'
import graphql from 'highlight.js/lib/languages/graphql'
import javascript from 'highlight.js/lib/languages/javascript'
import json from 'highlight.js/lib/languages/json'
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

;[['asm', x86asm], ['bash', bash], ['c', c], ['csharp', csharp], ['css', css], ['dockerfile', dockerfile], ['fsharp', fsharp], ['graphql', graphql], ['javascript', javascript], ['json', json], ['jsx', javascript], ['markdown', markdown], ['python', python], ['sql', sql], ['text', plaintext], ['tsx', typescript], ['typescript', typescript], ['yaml', yaml], ['html', xml]].forEach(([name, language]) => hljs.registerLanguage(name, language))

const REPO = 'https://github.com/xiehanff/trans-books-city'
const BOOKS = [
  {
    slug: 'computer-systems-3e',
    title: 'Computer Systems: A Programmer’s Perspective',
    zh: '深入理解计算机系统（原书第 3 版）',
    label: 'COMPUTER SYSTEMS',
    author: 'Randal E. Bryant / David R. O’Hallaron',
    cover: coverCsapp,
    desc: '从程序表示与执行，到链接、异常控制流、虚拟内存、网络与并发，系统理解计算机系统的软硬件基础。',
  },
  {
    slug: 'operating-systems-three-easy-pieces',
    title: 'Operating Systems: Three Easy Pieces',
    zh: '操作系统导论',
    label: 'OPERATING SYSTEMS',
    author: 'Remzi H. Arpaci-Dusseau / Andrea C. Arpaci-Dusseau',
    cover: coverOstep,
    desc: '以虚拟化、并发、持久性三大主题贯穿进程、调度、内存、文件系统与分布式，对话式讲解操作系统核心原理。',
  },
  {
    slug: 'structure-and-interpretation-of-computer-programs',
    title: 'Structure and Interpretation of Computer Programs',
    zh: '计算机程序的构造和解释（第 2 版）',
    label: 'SICP',
    author: 'Harold Abelson / Gerald Jay Sussman / Julie Sussman',
    cover: coverSicp,
    desc: '用 Scheme 讲透过程抽象、数据抽象、模块化与元语言抽象，程序设计的经典之作，裘宗燕译本。',
  },
  {
    slug: 'computer-networking-top-down-6e',
    title: 'Computer Networking: A Top-Down Approach',
    zh: '计算机网络：自顶向下方法（原书第 6 版）',
    label: 'COMPUTER NETWORKING',
    author: 'James F. Kurose / Keith W. Ross',
    cover: coverComputerNetworking,
    desc: '从应用层一路向下，讲透运输层、网络层与链路层，配 Wireshark 实验与编程作业的自顶向下网络经典教材。',
  },
]

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

const md = new MarkdownIt({
  html: true,
  linkify: true,
  typographer: true,
  highlight(code, language) {
    if (language && hljs.getLanguage(language)) return `<pre class="hljs"><code>${hljs.highlight(code, { language }).value}</code></pre>`
    return `<pre class="hljs"><code>${md.utils.escapeHtml(code)}</code></pre>`
  },
}).use(markdownItAnchor, { permalink: false })
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
  return md.render(source, { doc }).replace(/<p>@@MATH(\d+)@@<\/p>/g, (_, index) => `<div class="math-display">${math[Number(index)]}</div>`)
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
    { title: '正文', test: (doc) => doc.number > 0 && !/附录|勘误|API时效|术语表|更多资源|参考文献/.test(doc.file) },
    { title: '附录', test: (doc) => doc.number > 0 && /附录|勘误|API时效|术语表|更多资源|参考文献/.test(doc.file) },
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
    if (activeBookSlug.value) goHome()
    return
  }
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
    <div class="top-actions"><span class="edition">中文译本 · 精选技术书籍</span><a :href="REPO" target="_blank" rel="noreferrer" aria-label="GitHub">GitHub ↗</a></div>
  </header>
  <main v-if="!activeBook" class="home">
    <div class="home-wrap">
      <section class="home-hero">
        <h1>Trans Books City</h1>
        <p>{{ BOOKS.length }} 本精选技术书籍的中文版，译文经逐章评审与原书核对。</p>
      </section>
      <section class="book-grid">
        <button v-for="book in BOOKS" :key="book.slug" class="book-card" @click="openBook(book)">
          <img class="book-cover" :src="book.cover" :alt="`《${book.title}》封面`" loading="lazy" />
          <div class="book-info">
            <h2 class="book-title">{{ book.title }}</h2>
            <p class="book-zh">{{ book.zh }} · 中文版</p>
            <p class="book-desc">{{ book.desc }}</p>
            <p class="book-meta">{{ book.author }} · {{ book.chapters.length }} 章</p>
          </div>
        </button>
      </section>
      <footer class="home-foot">译本仅供学习交流，版权归原作者所有 · <a :href="REPO">在 GitHub 上查看 ↗</a></footer>
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
      <div class="sidebar-foot">{{ activeBook.title }} · 中文译本<br /><a :href="REPO">在 GitHub 上查看 ↗</a></div>
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
        <footer class="article-footer"><span>译本仅供学习交流，版权归原作者所有</span><a :href="REPO">GitHub ↗</a></footer>
      </div>
    </main>
  </div>
</template>
