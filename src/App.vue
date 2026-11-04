<script setup>
import { computed, reactive, ref } from 'vue'
import MarkdownIt from 'markdown-it'
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
import coverDomainModeling from './assets/covers/domain-modeling-made-functional.jpg'
import coverCompleteDeveloper from './assets/covers/the-complete-developer.jpg'
import coverWebBrowser from './assets/covers/web-browser-engineering.jpg'
import coverWritingC from './assets/covers/writing-a-c-compiler.jpg'
import coverAlgorithmDesignManual from './assets/covers/algorithm-design-manual.jpg'

;[['asm', x86asm], ['bash', bash], ['c', c], ['csharp', csharp], ['css', css], ['dockerfile', dockerfile], ['fsharp', fsharp], ['graphql', graphql], ['javascript', javascript], ['json', json], ['jsx', javascript], ['markdown', markdown], ['python', python], ['sql', sql], ['text', plaintext], ['tsx', typescript], ['typescript', typescript], ['yaml', yaml], ['html', xml]].forEach(([name, language]) => hljs.registerLanguage(name, language))

const REPO = 'https://github.com/xiehanff/trans-books-city'
const BOOKS = [
  {
    slug: 'the-algorithm-design-manual',
    title: 'The Algorithm Design Manual',
    zh: '算法设计指南',
    label: 'THE ALGORITHM DESIGN MANUAL',
    author: 'Steven S. Skiena',
    cover: coverAlgorithmDesignManual,
    desc: '从算法分析、数据结构、排序与查找，到图算法、动态规划和近似算法，系统学习经典算法设计方法。',
  },
  {
    slug: 'domain-modeling-made-functional',
    title: 'Domain Modeling Made Functional',
    zh: '领域建模函数式',
    label: 'DOMAIN MODELING MADE FUNCTIONAL',
    author: 'Scott Wlaschin',
    cover: coverDomainModeling,
    desc: '函数式编程是领域建模的绝佳选择——用 F# 的类型与管道，产出既清晰又简洁的领域设计。',
  },
  {
    slug: 'the-complete-developer',
    title: 'The Complete Developer',
    zh: '全栈开发者',
    label: 'THE COMPLETE DEVELOPER',
    author: 'Martin Krause',
    cover: coverCompleteDeveloper,
    desc: '从 Node.js、TypeScript、React 到 GraphQL、OAuth 与 Docker，一次打通现代全栈 Web 开发。',
  },
  {
    slug: 'web-browser-engineering',
    title: 'Web Browser Engineering',
    zh: '浏览器工程',
    label: 'WEB BROWSER ENGINEERING',
    author: 'Pavel Panchekha / Chris Harrelson',
    cover: coverWebBrowser,
    desc: '从下载网页、解析 HTML、排版绘制到运行 JavaScript，亲手构建一个浏览器，驱散黑盒的神秘感。',
  },
  {
    slug: 'writing-a-c-compiler',
    title: 'Writing a C Compiler',
    zh: '动手写 C 编译器',
    label: 'WRITING A C COMPILER',
    author: 'Nora Sandler',
    cover: coverWritingC,
    desc: '从返回常量到结构体与寄存器分配，用 20 章一步步实现一个真实可用的 C 编译器，吃透 C 语言与 x86-64 汇编。',
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
        // 部分文件以“第 X 部分”扉页开头，加载后优先用章级标题作导航名称。
        const chapter = content.match(/^#{1,4}[ \t]+第\s*[0-9]+\s*章[^\n]*$|^#{1,4}[ \t]+导言[^\n]*$/m)?.[0]
        const title = (chapter || content.match(/^#\s+(.+)$/m)?.[1] || doc.title).replace(/^#{1,4}[ \t]+/, '').replace(/`/g, '')
        doc.title = title
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
function activateDoc(doc) {
  if (doc.book !== activeBookSlug.value) query.value = ''
  activeBookSlug.value = doc.book
  activeId.value = doc.id
  menuOpen.value = false
  window.scrollTo(0, 0)
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
function syncFromHash() {
  const id = decodeURIComponent(location.hash.slice(1)).replace(/^\//, '')
  if (!id) {
    if (activeBookSlug.value) goHome()
    return
  }
  const [slug, file] = id.split('/')
  const book = BOOKS.find((item) => item.slug === slug)
  const doc = book ? book.chapters.find((item) => item.file === file) : null
  if (!doc) return
  activateDoc(doc)
}
syncFromHash()
window.addEventListener('hashchange', syncFromHash)
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
        <p>{{ BOOKS.length }} 本英文技术书籍的中文译本，译文经逐章评审与原书勘误核对。</p>
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
